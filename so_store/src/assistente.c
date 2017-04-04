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
#include "memoria.h"
#include "assistente.h"

int assistente_executar(int id) {
	int ret;
	struct produto Produto;
	int clientes_atendidos_assistente = 0;

	setbuf(stdout, NULL);

	// Passar pedidos de produtos para a loja
	while (1) {
		ret = memoria_pedido_p_le(id, &Produto);
		if (ret == 1) {
			// produto disponível
			memoria_pedido_e_escreve(id, &Produto);
		} else if (ret == 2) {
			// produto indisponível
			memoria_recibo_r_escreve(id, &Produto);
		} else
			// sem clientes
			break;
		clientes_atendidos_assistente++;
	}

	return clientes_atendidos_assistente;
}
