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

#include "loja.h"
#include "main.h"
#include "memoria.h"
#include "tempo.h"

extern struct indicadores Ind;
extern struct configuracao Config;

int loja_executar(int id) {
	struct produto Produto;
	int clientes_atendidos_pela_loja = 0;

	setbuf(stdout, NULL);

	// Preparar produtos
	while (1) {
		// aguardar pedido da assistente
		if (memoria_pedido_e_le(id, &Produto) == 0)
			break;
		// preparar produto e aguardar processamento de encomenda
		tempo_processamento_encomenda();
		// colocar recibo
		memoria_recibo_r_escreve(id, &Produto);
		Ind.produtos_entregues_pelas_lojas[Produto.id * Config.LOJAS + id]++;
		clientes_atendidos_pela_loja++;
	}
	//    printf("LOJA %02d terminou\n",id);
	return clientes_atendidos_pela_loja;
}
