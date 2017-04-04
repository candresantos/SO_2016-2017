#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <semaphore.h>
#include <time.h>
#include <pthread.h>
#include <errno.h>
#include <sys/mman.h> //mmap
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <signal.h>
#include <limits.h>

#include "main.h"
#include "so.h"
#include "memoria.h"
#include "prodcons.h"
#include "controlo.h"
#include "escalonador.h"
#include "ficheiro.h"
#include "tempo.h"

//==============================================
// DECLARAR ACESSO A DADOS EXTERNOS
//
extern struct configuracao Config;
//==============================================

struct recibo_r BRecibo;  // buffer loja-cliente
struct pedido_e BEncomenda; // buffer assistente-loja
struct pedido_p BProduto; 	// buffer cliente-assistente

//******************************************
// CRIAR ZONAS DE MEMORIA
//
void * memoria_criar(char * name, int size) {
	//==============================================
	// FUNÇÃO GENÉRICA DE CRIAÇÃO DE MEMÓRIA PARTILHADA
	//
	// usar getuid() para gerar nome unico na forma:
	// sprintf(name_uid,"/%s_%d", name, getuid())
	// usar name_uid em shm_open
	// usar tambem: ftruncate e mmap
	return so_memoria_criar(name, size);
	//==============================================
}
void memoria_criar_stock() {
	//==============================================
	// CRIAR ZONA DE MEMÓRIA PARA O STOCK DE CADO PRODUTO
	//
	// utilizar a função genérica memoria_criar(char *,int)
	// para criar ponteiro que se guarda em Config.stock
	// que deve ter capacidade para um vetor unidimensional
	// com a dimensao [PRODUTOS] para inteiro
	so_memoria_criar_stock();
	//==============================================
}
void memoria_criar_buffers() {
	//==============================================
	// CRIAR ZONAS DE MEMÓRIA PARA OS BUFFERS: PEDIDOS DE PRODUTO, PEDIDOS DE ENCOMENDA e RECIBOS
	//
	// utilizar a função genérica memoria_criar(char *,int)
	//
	// para criar ponteiro que se guarda em BRecibo.ptr
	// que deve ter capacidade para um vetor unidimensional
	// de inteiros com a dimensao [BUFFER_RECIBO]
	// para criar ponteiro que se guarda em BRecibo.buffer
	// que deve ter capacidade para um vetor unidimensional
	// com a dimensao [BUFFER_RECIBO] para struct produto
	//
	// para criar ponteiro que se guarda em BEncomenda.ptr
	// que deve ter capacidade para um vetor unidimensional
	// de inteiros com a dimensao [BUFFER_ENCOMENDA]
	// para criar ponteiro que se guarda em BEncomenda.buffer
	// que deve ter capacidade para um vetor unidimensional
	// com a dimensao [BUFFER_ENCOMENDA] para struct produto
	//
	// para criar ponteiro que se guarda em BProduto.ptr
	// que deve ter capacidade para struct ponteiros
	// para criar ponteiro que se guarda em BProduto.buffer
	// que deve ter capacidade para um vetor unidimensional
	// com a dimensao [BUFFER_PEDIDO] para struct produto
	so_memoria_criar_buffers();
	//==============================================
}
void memoria_criar_escalonador() {
	//==============================================
	// CRIAR ZONA DE MEMÓRIA PARA O MAPA DE ESCALONAMENTO
	//
	// utilizar a função genérica memoria_criar(char *,int)
	// para criar ponteiro que se guarda em Escalonamento.ptr
	// que deve ter capacidade para um vetor bidimensional
	// com a dimensao [PRODUTOS,LOJAS] para inteiro
	so_memoria_criar_escalonador();
	//==============================================
}

void memoria_terminar(char * name, void * ptr, int size) {
	//==============================================
	// FUNÇÃO GENÉRICA DE DESTRUIÇÃO DE MEMÓRIA PARTILHADA
	//
	// usar getuid() para gerar nome unico na forma:
	// sprintf(name_uid,"/%s_%d", name, getuid())
	// usar name_uid em shm_unlink
	// usar tambem: munmap
	so_memoria_terminar(name, ptr, size);
	//==============================================
}

//******************************************
// MEMORIA_DESTRUIR
//
void memoria_destruir() {
	//==============================================
	// DESTRUIR MAPEAMENTO E NOME DE PÁGINAS DE MEMÓRIA
	//
	// utilizar a função genérica memoria_terminar(char *,void *,int)
	so_memoria_destruir();
	//==============================================
}

//******************************************
// MEMORIA_PEDIDO_P_ESCREVE
//
void memoria_pedido_p_escreve(int id, struct produto *pProduto) {
	prodcons_pedido_p_produzir_inicio();

	// registar hora do pedido de PRODUTO
	tempo_registar(&pProduto->hora_pedido);

	//==============================================
	// ESCREVER PEDIDO DE PRODUTO NO BUFFER PEDIDO DE PRODUTOS
	//
	// copiar conteudo relevante da estrutura pProduto para
	// a posicao BProduto.ptr->in do buffer BProduto
	// conteudo: cliente, id, hora_pedido
	// colocar disponivel = 1 nessa posicao do BProduto
	// e atualizar BProduto.ptr->in
	so_memoria_pedido_p_escreve(id, pProduto);
	//==============================================

	prodcons_pedido_p_produzir_fim();

	// informar ASSISTENTE de pedido de PRODUTO
	controlo_cliente_submete_pedido(id);

	// log
	ficheiro_escrever_log_ficheiro(1, id);
}
//******************************************
// MEMORIA_PEDIDO_P_LE
//
int memoria_pedido_p_le(int id, struct produto *pProduto) {
	// testar se existem clientes e se o SO_STORE esta aberto
	if (controlo_assistente_aguarda_pedido(id) == 0)
		return 0;

	prodcons_pedido_p_consumir_inicio();

	//==============================================
	// LER PEDIDO DE PRODUTO DO BUFFER PEDIDO DE PRODUTOS
	//
	// copiar conteudo relevante da posicao BProduto.ptr->out 
	// do buffer BProduto para a estrutura pProduto
	// conteudo: cliente, id, hora_pedido
	// colocar disponivel = 0 nessa posicao do BProduto
	// e atualizar BProduto.ptr->out
	so_memoria_pedido_p_le(id, pProduto);
	//==============================================

	// testar se existe stock do PRODUTO pedido pelo cliente
	if (prodcons_atualizar_stock(pProduto->id) == 0) {
		pProduto->disponivel = 0;
		prodcons_pedido_p_consumir_fim();
		return 2;
	} else
		pProduto->disponivel = 1;

	prodcons_pedido_p_consumir_fim();

	// log
	ficheiro_escrever_log_ficheiro(2, id);

	return 1;
}

//******************************************
// MEMORIA_PEDIDO_E_ESCREVE
//
void memoria_pedido_e_escreve(int id, struct produto *pProduto) {
	int pos, loja;

	prodcons_pedido_e_produzir_inicio();

	// decidir a que loja se destina
	loja = escalonador_obter_loja(id, pProduto->id);

	//==============================================
	// ESCREVER PEDIDO NO BUFFER DE PEDIDOS DE ENCOMENDA
	//
	// procurar posicao vazia no buffer BEncomenda
	// copiar conteudo relevante da estrutura pProduto para
	// a posicao BEncomenda.ptr->in do buffer BEncomenda
	// conteudo: cliente, id, disponivel, assistente, loja, hora_pedido
	// colocar BEncomenda.ptr[n] = 1 na posicao respetiva
	pos = so_memoria_pedido_e_escreve(id, pProduto, loja);
	//==============================================

	prodcons_pedido_e_produzir_fim();

	// informar loja respetiva de pedido de encomenda
	controlo_assistente_submete_pedido(loja);

	// registar hora pedido (encomenda)
	tempo_registar(&BRecibo.buffer[pos].hora_encomenda);

	// log
	ficheiro_escrever_log_ficheiro(3, id);
}
//******************************************
// MEMORIA_PEDIDO_E_LE
//
int memoria_pedido_e_le(int id, struct produto *pProduto) {
	// testar se existem pedidos e se o SO_Store esta aberto
	if (controlo_loja_aguarda_pedido(id) == 0)
		return 0;

	prodcons_pedido_e_consumir_inicio();

	//==============================================
	// LER PEDIDO DO BUFFER DE PEDIDOS DE ENCOMENDA
	//
	// procurar pedido de encomenda para a loja no buffer BEncomenda
	// copiar conteudo relevante da posicao encontrada
	// no buffer BEncomenda para pProduto
	// conteudo: cliente, id, disponivel, assistente, hora_pedido, hora_encomenda
	// colocar BEncomenda.ptr[n] = 0 na posicao respetiva
	so_memoria_pedido_e_le(id, pProduto);
	//==============================================

	prodcons_pedido_e_consumir_fim();

	// log
	ficheiro_escrever_log_ficheiro(4, id);

	return 1;
}

//******************************************
// MEMORIA_RECIBO_R_ESCREVE
//
void memoria_recibo_r_escreve(int id, struct produto *pProduto) {
	int pos;

	prodcons_recibo_r_produzir_inicio();

	//==============================================
	// ESCREVER RECIBO NO BUFFER DE RECIBOS
	//
	// procurar posicao vazia no buffer BRecibo
	// copiar conteudo relevante da estrutura pProduto para
	// a posicao BRecibo.ptr->in do buffer BRecibo
	// conteudo: cliente, id, disponivel, assistente, loja, hora_pedido, hora_encomenda
	// colocar BRecibo.ptr[n] = 1 na posicao respetiva
	pos = so_memoria_recibo_r_escreve(id, pProduto);
	//==============================================

	prodcons_recibo_r_produzir_fim();

	// informar cliente de que o recibo esta pronto
	controlo_loja_submete_recibo(pProduto->cliente);

	// registar hora pronta (recibo)
	tempo_registar(&BRecibo.buffer[pos].hora_recibo);

	// log
	ficheiro_escrever_log_ficheiro(5, id);
}
//******************************************
// MEMORIA_RECIBO_R_LE
//
void memoria_recibo_r_le(int id, struct produto *pProduto) {
	// aguardar recibo
	controlo_cliente_aguarda_recibo(pProduto->cliente);

	prodcons_recibo_r_consumir_inicio();

	//==============================================
	// LER RECIBO DO BUFFER DE RECIBOS
	//
	// procurar recibo para o cliente no buffer BRecibo
	// copiar conteudo relevante da posicao encontrada
	// no buffer BRecibo para pProduto
	// conteudo: cliente, disponivel, assistente, loja, hora_pedido, hora_encomenda, hora_recibo
	// colocar BRecibo.ptr[n] = 0 na posicao respetiva
	so_memoria_recibo_r_le(id, pProduto);
	//==============================================

	prodcons_recibo_r_consumir_fim();

	// log
	ficheiro_escrever_log_ficheiro(6, id);
}

//******************************************
// MEMORIA_CRIAR_INDICADORES
//
void memoria_criar_indicadores() {
	//==============================================
	// CRIAR ZONAS DE MEMÓRIA PARA OS INDICADORES
	//
	// criação dinâmica de memória
	// para cada campo da estrutura indicadores
	so_memoria_criar_indicadores();
	// iniciar indicadores relevantes:
	// - Ind.stock_inicial (c/ Config.stock respetivo)
	// - Ind.clientes_atendidos_pelas_assistentes (c/ 0)
	// - Ind.clientes_atendidos_pelas_lojas (c/ 0)
	// - Ind.produtos_obtidos_pelos_clientes (c/ 0)
	so_memoria_iniciar_indicadores();
	//==============================================
}

