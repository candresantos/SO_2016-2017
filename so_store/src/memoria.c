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
	return so_memoria_criar(name, size);
	//==============================================
}
void memoria_criar_stock() {
	//==============================================
	// CRIAR ZONA DE MEMÓRIA PARA O STOCK DE CADO PRODUTO
	//
	// utilizar a função genérica memoria_criar(char *,int)
	so_memoria_criar_stock();
	//==============================================
}
void memoria_criar_buffers() {
	//==============================================
	// CRIAR ZONAS DE MEMÓRIA PARA OS BUFFERS: PEDIDOS DE PRODUTO, PEDIDOS DE ENCOMENDA e RECIBOS
	//
	// utilizar a função genérica memoria_criar(char *,int)
	so_memoria_criar_buffers();
	//==============================================
}
void memoria_criar_escalonador() {
	//==============================================
	// CRIAR ZONA DE MEMÓRIA PARA O MAPA DE ESCALONAMENTO
	//
	// utilizar a função genérica memoria_criar(char *,int)
	so_memoria_criar_escalonador();
	//==============================================
}

void memoria_terminar(char * name, void * ptr, int size) {
	//==============================================
	// FUNÇÃO GENÉRICA DE DESTRUIÇÃO DE MEMÓRIA PARTILHADA
	//
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
// MEMORIA_PEDIDO_S_ESCREVE
//
void memoria_pedido_p_escreve(int id, struct produto *pProduto) {
	prodcons_pedido_p_produzir_inicio();

	// registar hora do pedido de PRODUTO
	tempo_registar(&pProduto->hora_pedido);

	//==============================================
	// ESCREVER PEDIDO DE PRODUTO NO BUFFER PEDIDO DE PRODUTOS
	//
	so_memoria_pedido_p_escreve(id, pProduto);
	//==============================================

	prodcons_pedido_p_produzir_fim();

	// informar ASSISTENTE de pedido de PRODUTO
	controlo_cliente_submete_pedido(id);

	// log
	ficheiro_escrever_log_ficheiro(1, id);
}
//******************************************
// MEMORIA_PEDIDO_S_LE
//
int memoria_pedido_p_le(int id, struct produto *pProduto) {
	// testar se existem clientes e se o SO_STORE esta aberto
	if (controlo_assistente_aguarda_pedido(id) == 0)
		return 0;

	prodcons_pedido_p_consumir_inicio();

	//==============================================
	// LER PEDIDO DE PRODUTO DO BUFFER PEDIDO DE PRODUTOS
	//
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
// MEMORIA_PEDIDO_I_ESCREVE
//
void memoria_pedido_e_escreve(int id, struct produto *pProduto) {
	int pos, loja;

	prodcons_pedido_e_produzir_inicio();

	// decidir a que loja se destina
	loja = escalonador_obter_loja(id, pProduto->id);

	//==============================================
	// ESCREVER PEDIDO NO BUFFER DE PEDIDOS DE ENCOMENDA
	//
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
// MEMORIA_PEDIDO_LE
//
int memoria_pedido_e_le(int id, struct produto *pProduto) {
	// testar se existem pedidos e se o SO_Store esta aberto
	if (controlo_loja_aguarda_pedido(id) == 0)
		return 0;

	prodcons_pedido_e_consumir_inicio();

	//==============================================
	// LER PEDIDO DO BUFFER DE PEDIDOS DE ENCOMENDA
	//
	so_memoria_pedido_e_le(id, pProduto);
	//==============================================

	prodcons_pedido_e_consumir_fim();

	// log
	ficheiro_escrever_log_ficheiro(4, id);

	return 1;
}

//******************************************
// MEMORIA_RECIBO_C_ESCREVE
//
void memoria_recibo_r_escreve(int id, struct produto *pProduto) {
	int pos;

	prodcons_recibo_r_produzir_inicio();

	//==============================================
	// ESCREVER RECIBO NO BUFFER DE RECIBOS
	//
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
// MEMORIA_RECIBO_C_LE
//
void memoria_recibo_r_le(int id, struct produto *pProduto) {
	int n;

	// aguardar recibo
	controlo_cliente_aguarda_recibo(pProduto->cliente);

	prodcons_recibo_r_consumir_inicio();

	//==============================================
	// LER RECIBO DO BUFFER DE RECIBOS
	//
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
	// - Ind.stock_inicial
	// - Ind.clientes_atendidos_pelas_assistentes
	// - Ind.clientes_atendidos_pelas_lojas
	// - Ind.produtos_obtidos_pelos_clientes
	so_memoria_iniciar_indicadores();
	//==============================================
}

