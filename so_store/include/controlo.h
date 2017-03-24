#ifndef CONTROLO_H_GUARD
#define CONTROLO_H_GUARD

// estrutura de condições para controlar o encerramento do SO_Store
struct controlo {
	pthread_mutex_t assistentes_mutex;
	pthread_cond_t assistentes_cond;
	pthread_mutex_t lojas_mutex;
	pthread_cond_t lojas_cond;
	// semaforo para o cliente informar as assistentes de uma encomenda
	sem_t *assistente_pedido;
	// semaforos para a assistente informar a loja de um pedido
	sem_t **loja_pedido;
	// semaforos para a loja informar o cliente de um recibo
	sem_t **cliente_recibo;
	int aberto;  // 0: se SO_Store fechado, 1: se SO_Store aberto
};

void controlo_criar();
void controlo_destruir();
void controlo_lock();
void controlo_unlock();
void controlo_aguarda_pedido();
void controlo_acorda_lojas();
void controlo_abrir_sostore();
void controlo_fechar_sostore();
void cond_criar(pthread_cond_t *cond, pthread_mutex_t *mutex);

void controlo_cliente_submete_pedido(int);
int controlo_assistente_aguarda_pedido(int);
void controlo_assistente_submete_pedido(int);
int controlo_loja_aguarda_pedido(int);
void controlo_loja_submete_recibo(int);
void controlo_cliente_aguarda_recibo(int);

#endif
