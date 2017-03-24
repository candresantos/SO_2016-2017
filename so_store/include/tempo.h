#ifndef TEMPO_H_GUARD
#define TEMPO_H_GUARD

extern struct configuracao Config; // configuração da execução do SO_Store

void tempo_iniciar(long);
void tempo_armar_alarme();
void tempo_escrever_log_temporizado(int);
double tempo_diferenca(struct timespec, struct timespec);
double tempo_ate_agora();
void tempo_registar(struct timespec *);
void tempo_processamento_encomenda();

#endif
