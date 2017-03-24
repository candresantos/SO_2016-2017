#ifndef MEMORIA_H_GUARD
#define MEMORIA_H_GUARD

// estrutura que contêm os ponteiros para um buffer circular
struct ponteiros {
	int in;  // ponteiro de escrita
	int out; // ponteiro de leitura
};

// estrutura onde são guardadas os pedidos de produtos do cliente para a assistente
struct pedido_p {
	struct produto *buffer;   // ponteiro para a lista de pedidos de produto
	struct ponteiros *ptr; // ponteiro para a estrutura de índices de escrita e leitura
};

// estrutura onde são guardados os pedidos de encomenda da assistente para a loja
struct pedido_e {
	struct produto *buffer; // ponteiro para a lista de pedidos de encomenda
	int *ptr;             // ponteiro para a lista de inteiros que indicam
						  // se a posição respetiva está livre ou ocupada
};

// estrutura onde a loja disponibiliza os recibos
struct recibo_r {
	struct produto *buffer; // ponteiro para a lista de recibos
	int *ptr;             // ponteiro para a lista de inteiros que indicam
						  // se a posição respetiva está livre ou ocupada
};

void * memoria_criar(char*, int);
void memoria_criar_stock();
void memoria_criar_buffers();
void memoria_criar_escalonador();
void memoria_destruir();
void memoria_pedido_p_escreve(int, struct produto *);
int memoria_pedido_p_le(int, struct produto *);
void memoria_pedido_e_escreve(int, struct produto *);
int memoria_pedido_e_le(int, struct produto *);
void memoria_recibo_r_escreve(int, struct produto *);
void memoria_recibo_r_le(int, struct produto *);
void memoria_criar_indicadores();
void memoria_terminar(char * name, void * ptr, int size);

#endif
