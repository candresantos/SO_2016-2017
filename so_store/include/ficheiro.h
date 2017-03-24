#ifndef FICHEIRO_H_GUARD
#define FICHEIRO_H_GUARD

// estrutura de registo de nomes e handles para ficheiros
struct ficheiros {
	char *entrada;
	char *saida;
	char *log;
	FILE *h_entrada;
	FILE *h_saida;
	FILE *h_log;
};

void ficheiro_iniciar(char *, char *, char *);
void ficheiro_destruir();
void ficheiro_escrever_log_ficheiro(int, int);
void ficheiro_escrever_linha(char *);

static int handler(void*, const char*, const char*, const char*);

#endif
