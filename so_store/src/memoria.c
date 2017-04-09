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
extern struct escalonamento Escalonamento;
extern struct indicadores Ind;
//==============================================

struct recibo_r BRecibo;  // buffer loja-cliente
struct pedido_e BEncomenda; // buffer assistente-loja
struct pedido_p BProduto; 	// buffer cliente-assistente

//==============================================
// EXTRA FUNCTIONS
void bufferWriter(struct produto **, struct produto *, int, int);
void inc(int *, int);
int findIndex(int *in, struct produto **, int, int, int);
int randAccBufferWriter(struct produto **, struct produto *, int *, int,
						int);
void bufferReader(struct produto **, struct produto *, int, int);
void randAccBufferReader(struct produto **, struct produto *, int *, 
						int, int, int);
void freeInd();
void allocateInd();
void iniInd();
//******************************************
// CRIAR ZONAS DE MEMORIA
//
void * memoria_criar(char * name, int size) {
	//strlen + unsigned int possible values + /_
	char name_uid[strlen(name)+12];
	int fd = 0, ret = 0;
	void *ptr = NULL;
	//name_uid = /name_getuid()
	sprintf(name_uid,"/%s_%d",name,getuid());
	fd = shm_open(name_uid, O_RDWR | O_CREAT | O_TRUNC, S_IWUSR | S_IRUSR);
	if(fd == -1){ //failed to open
		perror("shm_open");
		exit(1);
	}
	ret = ftruncate(fd, size);
	if(ret == -1){ //failed
		perror("ftruncate");
		exit(1);
	}
	ptr = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if(ptr == MAP_FAILED){ //failed
		perror("map");
		exit(1);
	}
	
	return ptr;
}
void memoria_criar_stock() {
	//Create shared memory for stock.
	Config.stock = (int *)memoria_criar(STR_SHM_STOCK,
	                                  sizeof(int)*Config.PRODUTOS);
}
void memoria_criar_buffers() {
	//Create shared memory for every BRecibo field
	BRecibo.ptr = (int *) memoria_criar(STR_SHM_RECIBO_PTR, 
	                    sizeof(int)*Config.BUFFER_RECIBO);
	BRecibo.buffer = (struct produto *) memoria_criar(STR_SHM_RECIBO_BUFFER, 
						sizeof(struct produto)*Config.BUFFER_RECIBO);
	//Create shared memory for every BEncomenda field
	BEncomenda.ptr = (int *) memoria_criar(STR_SHM_ENCOMENDA_PTR,
						sizeof(int)*Config.BUFFER_ENCOMENDA);
	BEncomenda.buffer = (struct produto *)memoria_criar(STR_SHM_ENCOMENDA_BUFFER,
						sizeof(struct produto)*Config.BUFFER_ENCOMENDA);
	//Create shared memory for every BProduto field
	BProduto.ptr = (int *) memoria_criar(STR_SHM_PRODUTO_PTR,
						sizeof(struct ponteiros));
	BProduto.buffer = (struct produto *) memoria_criar(STR_SHM_PRODUTO_BUFFER,
						sizeof(struct produto)*Config.BUFFER_PEDIDO);
}
void memoria_criar_escalonador() {
	//Create shared memory for Escalonamento
	Escalonamento.ptr = (int *) memoria_criar(STR_SHM_ESCALONAMENTO, 
							sizeof(int)*Config.PRODUTOS*Config.LOJAS);
}

void memoria_terminar(char * name, void * ptr, int size) {
	int ret = 0;
	//strlen + unsigned int possible values + /_
	char name_uid[strlen(name)+12];
	ret = munmap(ptr,size);
	if(ret == -1){ //failed
		perror("munmap");
		exit(1);
	}
	//name_uid = /name_getuid
	sprintf(name_uid,"/%s_%d",name,getuid());
	ret = shm_unlink(name_uid);
	if(ret == -1){ //failed
		perror("shm_unlink");
		exit(1);
	}
}

//******************************************
// MEMORIA_DESTRUIR
//
void memoria_destruir() {
	//Unlink stock memory
	memoria_terminar(STR_SHM_STOCK, Config.stock,
	                    sizeof(int)*Config.PRODUTOS);
	//Unlink every BRecibo field memory                    
	memoria_terminar(STR_SHM_RECIBO_BUFFER, BRecibo.buffer,
	                    sizeof(struct produto)*Config.BUFFER_RECIBO);
	memoria_terminar(STR_SHM_RECIBO_PTR, BRecibo.ptr,
	                    sizeof(int)*Config.BUFFER_RECIBO);
	//Unlink every BEncomenda field memory                    
	memoria_terminar(STR_SHM_ENCOMENDA_PTR, BEncomenda.ptr,
	                    sizeof(int)*Config.BUFFER_RECIBO);
	memoria_terminar(STR_SHM_ENCOMENDA_BUFFER, BEncomenda.ptr,
	                    sizeof(struct produto)*Config.BUFFER_ENCOMENDA);
	//Unlink every BProduto field memory                    
	memoria_terminar(STR_SHM_PRODUTO_PTR, BProduto.ptr,
	                    sizeof(struct ponteiros));
	memoria_terminar(STR_SHM_PRODUTO_BUFFER, BProduto.ptr,
	                    sizeof(struct produto)*Config.BUFFER_PEDIDO);
	//Unlink Escalonamento memory                   
	memoria_terminar(STR_SHM_ESCALONAMENTO, Escalonamento.ptr,
	                    sizeof(int)*Config.PRODUTOS*Config.LOJAS);
	freeInd();
}

//******************************************
// MEMORIA_PEDIDO_P_ESCREVE
//
void memoria_pedido_p_escreve(int id, struct produto *pProduto) {
	prodcons_pedido_p_produzir_inicio();
	// registar hora do pedido de PRODUTO
	tempo_registar(&pProduto->hora_pedido);
	
	//Set cliente as id
	pProduto->cliente = id;
	//Write to the buffer
	bufferWriter(&BProduto.buffer, pProduto, BProduto.ptr->in, 0);
	//Update BProduto.ptr->in
	inc(&BProduto.ptr->in, Config.BUFFER_PEDIDO);

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

	//Read buffer and copy to pProduto
	bufferReader(&BProduto.buffer, pProduto, BProduto.ptr->out, 0);
	//Update BProduto.ptr->out
	inc(&BProduto.ptr->out, Config.BUFFER_PEDIDO);

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
	
	//Set loja as loja and assistente as id
	pProduto->loja = loja;
	pProduto->assistente = id;
	
	//Write to the buffer
	pos = randAccBufferWriter(&BEncomenda.buffer, pProduto, 
							BEncomenda.ptr, 1, Config.BUFFER_ENCOMENDA);

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

	//Read buffer
	randAccBufferReader(&BEncomenda.buffer, pProduto, 
						BEncomenda.ptr, 1, Config.BUFFER_ENCOMENDA, id);

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
	//Set loja as id
	if(pProduto->disponivel)
		pProduto->loja = id;
	else
		pProduto->assistente = id;
	//Write to buffer
	pos = randAccBufferWriter(&BRecibo.buffer, pProduto, 
								BRecibo.ptr, 2, Config.BUFFER_RECIBO);
	
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

	//Read buffer
	randAccBufferReader(&BRecibo.buffer, pProduto, 
						BRecibo.ptr, 2, Config.BUFFER_RECIBO, id);
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
	allocateInd();
	// iniciar indicadores relevantes:
	// - Ind.stock_inicial (c/ Config.stock respetivo)
	// - Ind.clientes_atendidos_pelas_assistentes (c/ 0)
	// - Ind.clientes_atendidos_pelas_lojas (c/ 0)
	// - Ind.produtos_obtidos_pelos_clientes (c/ 0)
	iniInd();
	//==============================================
}

/*
 * bufferWriter copies from toCopy to a given position in the buffer.
 * 
 * bufferWriter receives the buffer to copy to, what to copy, the
 * position to write in the buffer, and a mode:
 * 
 * mode < 1 -> Basic write
 * mode >= 1 && mode < 2 -> basic + loja + assistente
 * mode >= 2 -> mode >= 1 + hora_encomenda
 */
void bufferWriter(struct produto **buffer, struct produto *toCopy, 
			     int in, int mode){
	//Copy the basic
	(*buffer)[in].cliente = toCopy->cliente;
	(*buffer)[in].id = toCopy->id;
	(*buffer)[in].disponivel = toCopy->disponivel;
	(*buffer)[in].hora_pedido = toCopy->hora_pedido;
	if(mode >= 1){ //Copy loja and assistente
		(*buffer)[in].loja = toCopy->loja;
		(*buffer)[in].assistente = toCopy->assistente;
	}
	if(mode >= 2) //Copy hora_encomenda
		(*buffer)[in].hora_encomenda = toCopy->hora_encomenda;
}

/*
 * bufferReader copies from a given position in a given buffer to
 * toCopy.
 * 
 * bufferReader receives the buffer to copy from, where to copy to,
 * the position to actually copy, and a mode:
 * 
 * mode < 1 -> Basic reading
 * mode >= 1 && mode < 2 -> mode < 1 + disponivel + assistente + hora_encomenda
 * mode >= 2 -> mode >= 1 + loja + hora_recibo
 */
void bufferReader(struct produto **buffer, struct produto *toCopy,
				 int in, int mode){
	//Copy the basic
	toCopy->cliente = (*buffer)[in].cliente;
	toCopy->id = (*buffer)[in].id;
	toCopy->hora_pedido = (*buffer)[in].hora_pedido;
	if(mode >= 1){ //Copy assistente, disponivel and hora_encomenda
		toCopy->assistente = (*buffer)[in].assistente;
		toCopy->hora_encomenda = (*buffer)[in].hora_encomenda;
		toCopy->disponivel = (*buffer)[in].disponivel;
	}
	if(mode >= 2){ //Copy loja and hora_recibo
		toCopy->loja = (*buffer)[in].loja;
		toCopy->hora_recibo = (*buffer)[in].hora_recibo;
	}
}

/*
 * randAccBufferWriter copies to the first free position of a 
 * given buffer, a given produto, 
 * and returns the position it just copied to.
 * 
 * randAccBufferWriter receives the buffer to copy to, what to copy,
 * the current state of the buffer (free/used positions), 
 * the size of the buffer, and a mode:
 * 
 * mode >= 1 && mode < 2 -> basic coppying
 * mode >= 2 -> basic + hora_encomenda
 */
int randAccBufferWriter(struct produto **buffer, struct produto *toCopy,
						int *in, int mode, int size){
	//Look for free position
	int index = findIndex(in,NULL,0,0, size);
	//Write to buffer
	bufferWriter(&(*buffer), toCopy, index, mode);
	return index;
}

/*
 * randAccBufferReader copies to toCopy the position which contains
 * lookup.
 * 
 * randAccBufferReader receives the buffer to copy from, where to copy to,
 * the current state of the buffer (free/used positions),
 * the size of the buffer, a mode and what to look for
 * 
 * mode >= 1 && mode < 2 -> basic coppying
 * mode >= 2 -> basic + loja + hora_recibo
 */
void randAccBufferReader(struct produto **buffer, struct produto *toCopy,
						int *in, int mode, int size, int lookup){
	int loja = 0;
	if(mode == 1) 
		loja = 1;
	int index = findIndex(in,&(*buffer), lookup, loja, size);
	bufferReader(&(*buffer), toCopy, index, mode);
}

/*
 * findIndex finds the first occurence of a certain value, or the first
 * empty position.
 * 
 * findIndex receives the state of the buffer, the buffer, 
 * what to look for, and if it should look for a loja or not
 * 
 * buffer == NULL -> Look for empty
 * buffer != NULL -> Look for lookup in loja/cliente depending on 
 * loja/!loja
 */
int findIndex(int *in, struct produto **buffer,int lookup, int loja, 
															int size){
	int index = 0, found = 0;
	//it hasn't found a value yet
	while(!found && index < size){
		//Look for empty
		if(buffer == NULL){
			if(!in[index]){ //Found an empty location
				in[index] = 1;
				found = 1;
			}
		//Look for lookup
		} else {
			if(in[index]){ //Found a writen location
				if(loja && (*buffer)[index].loja == lookup){ 
					//Found loja
					in[index] = 0;
					found = 1;
				} else if (!loja && (*buffer)[index].cliente == lookup){ 
					//Found cliente
					in[index] = 0;
					found = 1;
				}
			}
		}
		index++;
	}
	return (index-1);
}

/*
 * Pure modulo operation (circular buffer position)
 */
void inc(int *i, int size){
	*i = (*i + 1) % size;
}

/*
 * Free every allocated memory in Ind
 */
void freeInd(){
	free(Ind.pid_lojas);
	free(Ind.pid_clientes);
	free(Ind.pid_assistentes);
	free(Ind.clientes_atendidos_pelas_lojas);
	free(Ind.clientes_atendidos_pelas_assistentes);
	memoria_terminar(STR_SHM_PRODLOJAS, Ind.produtos_entregues_pelas_lojas, Config.PRODUTOS*Config.LOJAS*sizeof(int));
	free(Ind.produtos_obtidos_pelos_clientes);
	free(Ind.stock_inicial);
}

/*
 * Allocate memory to every Ind field
 */
void allocateInd(){
	Ind.clientes_atendidos_pelas_assistentes = 
						(int *) calloc(Config.ASSISTENTES, sizeof(int));
	Ind.clientes_atendidos_pelas_lojas =
						(int *) calloc(Config.LOJAS, sizeof(int));
	Ind.produtos_obtidos_pelos_clientes =
						(int *) calloc(Config.PRODUTOS, sizeof(int));
	Ind.pid_assistentes = (int *) calloc(Config.ASSISTENTES, sizeof(int));
	Ind.pid_clientes = (int *) calloc(Config.CLIENTES, sizeof(int));
	Ind.pid_lojas = (int *) calloc(Config.LOJAS, sizeof(int));
	Ind.stock_inicial = (int *) calloc(Config.PRODUTOS, sizeof(int));
	Ind.produtos_entregues_pelas_lojas = (int *) memoria_criar(STR_SHM_PRODLOJAS, Config.PRODUTOS*Config.LOJAS*sizeof(int));
}

/*
 * Initialize stock_inicial
 */
void iniInd(){
	int i = 0, j = 0;
	for(i = 0; i < Config.PRODUTOS; i++){
		Ind.stock_inicial[i] = Config.stock[i];
	}
}
