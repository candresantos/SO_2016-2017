/*Made by: G25
 *Carlos Santos 47805
 *Vasco Teixeira 45156
 *João Serrano 47868
 */
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
#include "controlo.h"
#include "prodcons.h"

//==============================================
// DECLARAR ACESSO A DADOS EXTERNOS
//
extern struct configuracao Config;
//==============================================

struct prodcons ProdCons;

void sem_ini(void *, void *);
void sem_end(void*, void*);

//******************************************
// SEMAFORO_CRIAR
//
sem_t * semaforo_criar(char * name, int value) {
	char name_uid[strlen(name)+12];
	sprintf(name_uid,"/%s_%d", name, getuid());
	sem_t *sem = sem_open(name_uid,O_CREAT | O_EXCL,0xFFFFFFFF, value);
	if(sem == SEM_FAILED){
		perror(name);
		exit(1);
	}
	return sem;
}

void prodcons_criar_stock() {
	//Create stock semaphore
	ProdCons.stock_mutex = semaforo_criar(STR_SEM_STOCK_MUTEX,1);
}

void prodcons_criar_buffers() {
	//Create pedidos_p_buffer semaphores
	ProdCons.pedido_p_empty = semaforo_criar(STR_SEM_PEDIDO_EMPTY,
											Config.BUFFER_PEDIDO);
	ProdCons.pedido_p_full = semaforo_criar(STR_SEM_PEDIDO_FULL, 0);
	ProdCons.pedido_p_mutex = semaforo_criar(STR_SEM_PEDIDO_MUTEX, 1);
	//Create pedidos_e_buffer semaphores
	ProdCons.pedido_e_empty = semaforo_criar(STR_SEM_ENCOMENDA_EMPTY,
											Config.BUFFER_PEDIDO);
	ProdCons.pedido_e_full = semaforo_criar(STR_SEM_ENCOMENDA_FULL, 0);
	ProdCons.pedido_e_mutex = semaforo_criar(STR_SEM_ENCOMENDA_MUTEX, 1);
	//Create recibos_r_buffer semaphores
	ProdCons.recibos_r_empty = semaforo_criar(STR_SEM_RECIBO_EMPTY,
											Config.BUFFER_PEDIDO);
	ProdCons.recibos_r_full = semaforo_criar(STR_SEM_RECIBO_FULL, 0);
	ProdCons.recibos_r_mutex = semaforo_criar(STR_SEM_RECIBO_MUTEX, 1);
}

void semaforo_terminar(char * name, void * ptr) {
	char name_uid[strlen(name)+12];
	sprintf(name_uid,"/%s_%d", name, getuid());
	if(sem_close(ptr) == -1){
		perror("Sem close");
		exit(1);
	}
	if(sem_unlink(name_uid) == -1){
		perror("Sem close");
		exit(1);
	}
}

void prodcons_destruir() {
	//close and unlink pedidos_p_buffer semaphores
	semaforo_terminar(STR_SEM_PEDIDO_EMPTY, ProdCons.pedido_p_empty);
	semaforo_terminar(STR_SEM_PEDIDO_FULL, ProdCons.pedido_p_full);
	semaforo_terminar(STR_SEM_PEDIDO_MUTEX, ProdCons.pedido_p_mutex);
	//close and unlink pedidos_e_buffer semaphores
	semaforo_terminar(STR_SEM_ENCOMENDA_EMPTY, ProdCons.pedido_e_empty);
	semaforo_terminar(STR_SEM_ENCOMENDA_FULL, ProdCons.pedido_e_full);
	semaforo_terminar(STR_SEM_ENCOMENDA_MUTEX, ProdCons.pedido_e_mutex);
	//close and unlink recibos_r_buffer semaphores
	semaforo_terminar(STR_SEM_RECIBO_EMPTY, ProdCons.recibos_r_empty);
	semaforo_terminar(STR_SEM_RECIBO_FULL, ProdCons.recibos_r_full);
	semaforo_terminar(STR_SEM_RECIBO_MUTEX, ProdCons.recibos_r_mutex);
	//close and unlink stock semaphore
	semaforo_terminar(STR_SEM_STOCK_MUTEX, ProdCons.stock_mutex);
}

//******************************************
void prodcons_pedido_p_produzir_inicio() {
	sem_ini(ProdCons.pedido_p_empty, ProdCons.pedido_p_mutex);
}

//******************************************
void prodcons_pedido_p_produzir_fim() {
	sem_end(ProdCons.pedido_p_full, ProdCons.pedido_p_mutex);
}

//******************************************
void prodcons_pedido_p_consumir_inicio() {
	sem_ini(ProdCons.pedido_p_full, ProdCons.pedido_p_mutex);
}

//******************************************
void prodcons_pedido_p_consumir_fim() {
	sem_end(ProdCons.pedido_p_empty, ProdCons.pedido_p_mutex);
}

//******************************************
void prodcons_pedido_e_produzir_inicio() {
	sem_ini(ProdCons.pedido_e_empty, ProdCons.pedido_e_mutex);
}

//******************************************
void prodcons_pedido_e_produzir_fim() {
	sem_end(ProdCons.pedido_e_full, ProdCons.pedido_e_mutex);
}

//******************************************
void prodcons_pedido_e_consumir_inicio() {
	sem_ini(ProdCons.pedido_e_full, ProdCons.pedido_e_mutex);
}

//******************************************
void prodcons_pedido_e_consumir_fim() {
	sem_end(ProdCons.pedido_e_empty, ProdCons.pedido_e_mutex);
}

//******************************************
void prodcons_recibo_r_produzir_inicio() {
	sem_ini(ProdCons.recibos_r_empty, ProdCons.recibos_r_mutex);
}

//******************************************
void prodcons_recibo_r_produzir_fim() {
	sem_end(ProdCons.recibos_r_full, ProdCons.recibos_r_mutex);
}

//******************************************
void prodcons_recibo_r_consumir_inicio() {
	sem_ini(ProdCons.recibos_r_full, ProdCons.recibos_r_mutex);
}

//******************************************
void prodcons_recibo_r_consumir_fim() {
	sem_end(ProdCons.recibos_r_empty, ProdCons.recibos_r_mutex);
}

//******************************************
void prodcons_buffers_inicio() {
	sem_ini(NULL, ProdCons.recibos_r_mutex);
	sem_ini(NULL, ProdCons.pedido_e_mutex);
	sem_ini(NULL, ProdCons.pedido_p_mutex);
}

//******************************************
void prodcons_buffers_fim() {
	sem_end(NULL, ProdCons.pedido_p_mutex);
	sem_end(NULL, ProdCons.pedido_e_mutex);
	sem_end(NULL, ProdCons.recibos_r_mutex);
}

//******************************************
int prodcons_atualizar_stock(int produto) {
	sem_ini(NULL, ProdCons.stock_mutex);
	int ret = 0;
	if(Config.stock[produto]){
		ret = 1;
		Config.stock[produto]--;
	}
	sem_end(NULL, ProdCons.stock_mutex);
	return ret;
}

/*
 * sem_ini waits for ptr and mutex
 * if ptr == NULL, it will only wait for mutex
 */
void sem_ini(void *ptr, void *mutex){
	if(ptr != NULL){
		if(sem_wait(ptr) == -1){
			perror("sem_full");
			prodcons_destruir();
			exit(1);
		}
	}
	if(sem_wait(mutex) == -1){
		perror("mutex");
		prodcons_destruir();
		exit(1);
	}
}

/*
 * sem_end posts ptr and mutex
 * if ptr == NULL, it will only post mutex
 */
void sem_end(void *ptr, void *mutex){
	if(sem_post(mutex) == -1){
		perror("mutex");
		prodcons_destruir();
		exit(1);
	}
	if(ptr != NULL){
		if(sem_post(ptr) == -1){
			perror("sem_empty");
			prodcons_destruir();
			exit(1);
		}
	}
}
