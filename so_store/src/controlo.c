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

//==============================================
// DECLARAR ACESSO A DADOS EXTERNOS
//
extern struct configuracao Config;
//==============================================

struct controlo *Controlo;          // controlo da abertura do SO_Store

void cond_criar(pthread_cond_t *cond, pthread_mutex_t *mutex) {
	//==============================================
	// FUNÇÃO GENÉRICA DE CRIAÇÃO DE UMA CONDIÇÃO PARTILHÁVEL ENTRE PROCESSOS
	//
	so_cond_criar(cond, mutex);
	//==============================================
}

void controlo_criar() {
	//==============================================
	// CRIAÇÃO DE ZONA DE MEMÓRIA PARTILHADA
	//
	// Criar e iniciar variaveis de condicao e respetivos mutexes do Controlo
	so_controlo_criar();
	//==============================================

	//==============================================
	// CRIAÇÃO DE MECANISMOS DE CONTROLO DE ABERTURA DO SO_Store
	//
	// Criar e iniciar variaveis de condicao e respetivos mutexes do Controlo
	so_controlo_criar_abertura();
	//==============================================

	//==============================================
	// CRIAÇÃO DE MECANISMOS PARA LOJA ACORDAR CLIENTE A QUE SE DESTINA O RECIBO
	//
	// Criar e iniciar semáforos
	so_controlo_criar_acordar_cliente();
	//==============================================

	//==============================================
	// CRIAÇÃO DE MECANISMOS PARA ASSISTENTE ACORDAR LOJA A QUE SE DESTINA O PEDIDO DE ENCOMENDA
	//
	// Criar e iniciar semáforos
	so_controlo_criar_acordar_loja();
	//==============================================

	//==============================================
	// CRIAÇÃO DE MECANISMOS PARA CLIENTE ACORDAR ASSISTENTE A QUE SE DESTINA O PEDIDO DE PRODUTO
	//
	// Criar e iniciar semáforos
	so_controlo_criar_acordar_assistente();
	//==============================================

}

void cond_terminar(pthread_cond_t *cond, pthread_mutex_t *mutex) {
	//==============================================
	// FUNÇÃO GENÉRICA DE DESTRUIÇÃO DE VARIÁVEIS DE CONDIÇÃO
	//
	so_cond_terminar(cond, mutex);
	//==============================================
}

void controlo_destruir() {
	//==============================================
	// DESTRUIÇÃO DE MECANISMOS DE CONTROLO DE ABERTURA DO SO_Store
	//
	// utilizar a função genérica cond_terminar
	so_controlo_destruir_abertura();
	//==============================================

	//==============================================
	// DESTRUIR SEMÁFOROS E RESPETIVOS NOMES
	//
	// utilizar a função genérica prodcons_terminar
	so_controlo_destruir_acordar_cliente();
	so_controlo_destruir_acordar_loja();
	so_controlo_destruir_acordar_assistente();
	//==============================================

	//==============================================
	// DESTRUIR ZONA DE MEMÓRIA
	//
	// utilizar a função genérica memoria_terminar
	so_controlo_destruir();
	//==============================================
}

//***************************************************
// ABERTURA DO SO_Store
//
void controlo_abrir_sostore() {
	//==============================================
	// ABRIR SO_Store
	//
	so_controlo_abrir_sostore();
	//==============================================
}

void controlo_fechar_sostore() {
	//==============================================
	// FECHAR SO_Store
	//
	so_controlo_fechar_sostore();
	//==============================================
}

//***************************************************
// CONTROLO ENTRE CLIENTES/ASSISTENTES/LOJAS
//
void controlo_cliente_submete_pedido(int id) {
	//==============================================
	// SINALIZAR ASSISTENTE DA SUBMISSÃO DE PEDIDO DE PRODUTO
	//
	so_controlo_cliente_submete_pedido(id);
	//==============================================
}
//******************************************
int controlo_assistente_aguarda_pedido(int id) {
	//==============================================
	// ASSISTENTE AGUARDA SUBMISSÃO DE PEDIDO DE PRODUTO
	//
	// verificar também se SO_Store está aberto
	return so_controlo_assistente_aguarda_pedido(id);
	//==============================================
}
//******************************************
void controlo_assistente_submete_pedido(int id) {
	//==============================================
	// SINALIZAR LOJA DA SUBMISSÃO DE PEDIDO DE ENCOMENDA
	//
	so_controlo_assistente_submete_pedido(id);
	//==============================================
}
//******************************************
int controlo_loja_aguarda_pedido(int id) {
	//==============================================
	// LOJA AGUARDA SUBMISSÃO DE PEDIDO DE ENCOMENDA
	//
	// verificar também se SO_Store está aberto
	return so_controlo_loja_aguarda_pedido(id);
	//==============================================
}
//******************************************
void controlo_loja_submete_recibo(int id) {
	//==============================================
	// SINALIZAR CLIENTE DA SUBMISSÃO DE RECIBO
	//
	so_controlo_loja_submete_recibo(id);
	//==============================================
}
//******************************************
void controlo_cliente_aguarda_recibo(int id) {
	//==============================================
	// CLIENTE AGUARDA SUBMISSÃO DE RECIBO
	//
	so_controlo_cliente_aguarda_recibo(id);
	//==============================================
}
