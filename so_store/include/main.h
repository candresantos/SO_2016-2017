#ifndef MAIN_H_GUARD
#define MAIN_H_GUARD

// estrutura que representa um produto nas tres etapas: pedido do produto, pedido de encomenda e recibo
struct produto {
	int id;  		            	// identificador do tipo de produto pedido
	int disponivel;                 // stock: 0 - indisponivel, 1 - disponivel
	int cliente;     	            // id do cliente que encomendou o produto
	int assistente;    	        // id da assistente que atendeu o cliente
	int loja;  	            // id da loja que efetuou o produto
	struct timespec hora_pedido; // hora a que o produto foi pedido a assistente (registada pelo cliente)
	struct timespec hora_encomenda; // hora a que a encomenda foi pedida a loja (registada pela assistente)
	struct timespec hora_recibo; // hora a que o recibo foi produzido pela loja (registada pela loja)
};

// estrutura com valores dos parâmetros do ficheiro de teste
struct configuracao {
	char* lista_produtos;      	// linha com stock de cada produto
	char* lista_clientes;    	// linha com produp pretendido por cada cliente
	char* lista_assistentes; // linha com os assistentes (nomes nao interessam)
	char* lista_lojas; // linha com os produtos por loja separadas por virgula
	char* lista_buffers;     	// linha com capacidade dos tres buffers
	int PRODUTOS;            	// nº de produtos disponiveis
	int CLIENTES;          	// nº de clientes
	int ASSISTENTES;        // nº de assistentes
	int LOJAS;       	// nº de lojas
	int BUFFER_PEDIDO; 		// capacidade do buffer de pedidos de produto
	int BUFFER_ENCOMENDA;    // capacidade do buffer de pedidos de encomenda
	int BUFFER_RECIBO;     // capacidade do buffer de recibos
	int* stock;             	// vetor com capacidade por tipo de produto
};

// estrutura de registo de indicadores de funcionamento do SO_Store
struct indicadores {
	int *stock_inicial;
	int *pid_clientes;
	int *pid_assistentes;
	int *pid_lojas;
	int *clientes_atendidos_pelas_assistentes;
	int *clientes_atendidos_pelas_lojas;
	int *produtos_entregues_pelas_lojas;
	int *produtos_obtidos_pelos_clientes;
};

#endif
