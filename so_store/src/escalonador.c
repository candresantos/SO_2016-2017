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
#include "escalonador.h"

struct escalonamento Escalonamento; // estrutura de suporte às decisões de escalonamento

void escalonador_iniciar(int loja, int produto) {
	//==============================================
	// REGISTAR QUE A LOJA FORNECE ESTE PRODUTO
	//
	// escrever 1 na posição respetiva do vetor Escalonamento.ptr
	so_escalonador_iniciar(loja, produto);
	//==============================================
}

int escalonador_obter_loja(int assistente, int produto) {
	//==============================================
	// ESCOLHER LOJA PARA ENCOMENDA O PRODUTO
	//
	// dada uma produto, escolher uma das lojas que disponibilize o produto
	//
	// a informação do assistente que está a pedir uma loja
	// pode não ser necessária
	return so_escalonador_obter_loja(assistente, produto);
	//==============================================
}
