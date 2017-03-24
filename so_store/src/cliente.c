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
#include "tempo.h"
#include "cliente.h"
#include "ficheiro.h"

extern struct configuracao Config;

int cliente_executar(int id) {
	int n, count, ret;
	struct produto Produto;
	int produto_id;
	char buf[100];
	char *result;

	setbuf(stdout, NULL);

	n = 0;
	count = 0;
	result = Config.lista_clientes;
	while (n < id) {
		while (Config.lista_clientes[count++] != '\0')
			;
		result = &Config.lista_clientes[count];
		n++;
	}

	produto_id = atoi(result);
	Produto.id = produto_id;
	Produto.cliente = id;

	memoria_pedido_p_escreve(id, &Produto);
	memoria_recibo_r_le(id, &Produto);

	if (Produto.disponivel == 1) {
		printf(
				"     CLIENTE %03d encomendou %d e obteve %d (v:%02d c:%02d t:%.9fs)\n",
				id, produto_id, Produto.id, Produto.assistente, Produto.loja,
				tempo_diferenca(Produto.hora_pedido, Produto.hora_recibo));
		sprintf(buf, "     CLIENTE %03d encomendou %d e obteve %d\n", id,
				produto_id, Produto.id);
		ret = Produto.id;
	} else {
		printf("     CLIENTE %03d encomendou %d mas nao estava disponivel!\n",
				id, produto_id);
		sprintf(buf,
				"     CLIENTE %03d encomendou %d mas nao estava disponivel!\n",
				id, produto_id);
		ret = Config.PRODUTOS;
	}

	ficheiro_escrever_linha(buf);

	return ret;
}
