#ifndef ESCALONADOR_H_GUARD
#define ESCALONADOR_H_GUARD

// estrutura onde se encontram quais as lojas que podem fornecer cada produto (1 ou 0)
struct escalonamento {
	int *ptr; // ponteiro para matriz [PRODUTOS,LOJAS]
};

void escalonador_iniciar(int, int);
int escalonador_obter_loja(int, int);

#endif
