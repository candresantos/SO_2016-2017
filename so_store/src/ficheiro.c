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
#include <ctype.h>
#include <signal.h>
#include <limits.h>

#include "main.h"
#include "so.h"
#include "memoria.h"
#include "escalonador.h"
#include "ficheiro.h"
#include "prodcons.h"
#include "tempo.h"

//==============================================
// DECLARAR ACESSO A DADOS EXTERNOS
//
extern struct configuracao Config;
//==============================================

struct ficheiros Ficheiros; // informação sobre nomes e handles de ficheiros

void ficheiro_iniciar(char *fic_entrada, char *fic_saida, char *fic_log) {
	//==============================================
	// GUARDAR NOMES DOS FICHEIROS NA ESTRUTURA Ficheiros
	//
	so_ficheiro_iniciar(fic_entrada, fic_saida, fic_log);
	//==============================================

	//==============================================
	// ABRIR FICHEIRO DE ENTRADA
	//
	so_ficheiro_abrir_ficheiro_entrada();
	//==============================================

	// parse do ficheiro de teste
	// esta funcao prepara os campos da estrutura Config (char *)
	// com os dados do ficheiro de entrada
	if (ini_parse_file(Ficheiros.h_entrada, handler, &Config) < 0) {
		printf("Erro a carregar o ficheiro de teste'\n");
		exit(1);
	}

	// agora e' preciso inicializar os restantes campos da estrutura Config

	//==============================================
	// CONTAR PRODUTOS
	//
	so_ficheiro_contar_produtos();
	//==============================================

	// iniciar memoria para o vetor com o stock por PRODUTO e semaforo
	memoria_criar_stock();
	prodcons_criar_stock();

	//==============================================
	// LER STOCK DE CADO PRODUTO
	//
	so_ficheiro_ler_stock();
	//==============================================

	//==============================================
	// CONTAR CLIENTES
	//
	so_ficheiro_contar_clientes();
	//==============================================

	//==============================================
	// CONTAR ASSISTENTES
	//
	so_ficheiro_contar_assistentes();
	//==============================================

	//==============================================
	// CONTAR LOJAS
	//
	so_ficheiro_contar_lojas();
	//==============================================

	//==============================================
	// LER ESPECIALIDADES
	//
	so_ficheiro_ler_especialidades();
	//==============================================

	//==============================================
	// LER CAPACIDADES DOS BUFFERS
	//
	so_ficheiro_ler_capacidades();
	//==============================================

	//==============================================
	// ABRIR FICHEIRO DE SAIDA (se foi especificado)
	//
	so_ficheiro_abrir_ficheiro_saida();
	//==============================================

	//==============================================
	// ABRIR FICHEIRO DE LOG (se foi especificado)
	//
	so_ficheiro_abrir_ficheiro_log();
	//==============================================
}

void ficheiro_destruir() {
	//==============================================
	// DESTRUIR ZONAS DE MEMÓRIA RESERVADAS DINAMICAMENTE
	//
	so_ficheiro_destruir();
	//==============================================
}

void ficheiro_escrever_log_ficheiro(int etapa, int id) {
	double t_diff;

	if (Ficheiros.h_log != NULL) {

		prodcons_buffers_inicio();

		// guardar timestamp
		t_diff = tempo_ate_agora();

		//==============================================
		// ESCREVER DADOS NO FICHEIRO DE LOG
		//
		// o log deve seguir escrupulosamente o formato definido
		so_ficheiro_escrever_log_ficheiro(etapa, id, t_diff);
		//==============================================

		prodcons_buffers_fim();
	}
}

void ficheiro_escrever_linha(char * linha) {
	//==============================================
	// ESCREVER UMA LINHA NO FICHEIRO DE SAIDA
	//
	so_ficheiro_escrever_linha(linha);
	//==============================================
}

int stricmp(const char *s1, const char *s2) {
	if (s1 == NULL)
		return s2 == NULL ? 0 : -(*s2);
	if (s2 == NULL)
		return *s1;

	char c1, c2;
	while ((c1 = tolower(*s1)) == (c2 = tolower(*s2))) {
		if (*s1 == '\0')
			break;
		++s1;
		++s2;
	}

	return c1 - c2;
}

int handler(void* user, const char* section, const char* name,
		const char* value) {
	struct configuracao* pconfig = (struct configuracao*) user;

#define MATCH(s, n) stricmp(section, s) == 0 && stricmp(name, n) == 0
	if (MATCH("produtos", "stock")) {
		pconfig->lista_produtos = strdup(value);
	} else if (MATCH("clientes", "produto")) {
		pconfig->lista_clientes = strdup(value);
	} else if (MATCH("assistentes", "lista")) {
		pconfig->lista_assistentes = strdup(value);
	} else if (MATCH("lojas", "especialidades")) {
		pconfig->lista_lojas = strdup(value);
	} else if (MATCH("buffers", "capacidade")) {
		pconfig->lista_buffers = strdup(value);
	} else {
		return 0; /* unknown section/name, error */
	}
	return 1;
}

