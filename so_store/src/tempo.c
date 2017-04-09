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
#include "tempo.h"

struct timespec t_inicial;
long intervalo_alarme;

//==============================================
// DECLARAR ACESSO A DADOS EXTERNOS
//
extern struct configuracao Config;
//==============================================

struct timespec t_inicial;
long intervalo_alarme;

void tempo_iniciar(long intervalo) {
    //==============================================
    // INICIAR ESTRUTURA t_inicial COM VALOR DE RELOGIO (CLOCK_REALTIME)
	//
	// funções de tempo:
	// - clock_gettime() dá um resultado em nanosegundos
	// - gettimeofday()  dá um resultado em milisegundos
	// como a função clock_gettime() dá um valor mais preciso do que gettimeofday()
	// deve ser usada clock_gettime()
	//
    // fazer:
	// - se intervalo!=0 então intervalo_alarme = intervalo
	// - se intervalo!=0 então chamar tempo_armar_alarme();
	// - iniciar estrutura t_inicial com clock_gettime usando CLOCK_REALTIME
    so_tempo_iniciar(intervalo);
    //==============================================
}

void tempo_terminar(long intervalo)
{
    //==============================================
    // DESATIVAR ALARME
    //
	// desassociar SIGALRM da função tempo_escrever_log_temporizado
    so_tempo_terminar(intervalo);
    //==============================================
}

void tempo_armar_alarme() {
    //==============================================
    // ARMAR ALARME DE ACORDO COM intervalo_alarme (SIGNAL E SETTIMER)
    //
	// fazer:
	// - associar SIGALRM com a função tempo_escrever_log_temporizado
	// - usar setitimer preenchendo apenas os campos value da estrutura
    so_tempo_armar_alarme();
    //==============================================
}

void tempo_escrever_log_temporizado(int signum) {
    //==============================================
    // ESCREVER LOG NO ECRAN DE FORMA TEMPORIZADA 
    //
	// rearmar alarme chamando novamente tempo_armar_alarme
	// escrever para o ecrã a informação esperada
    so_tempo_escrever_log_temporizado(signum);
    //==============================================
}

double tempo_diferenca(struct timespec t1, struct timespec t2) {
    //==============================================
    // CALCULAR A DIFERENCA, EM NANOSEGUNDOS, ENTRE t1 E t2
    //
	// realizar as operações aritméticas necessárias para obter o resultado
    return so_tempo_diferenca(t1,t2);
    //==============================================
}

double tempo_ate_agora() {
    //==============================================
    // CALCULAR O INTERVALO DE TEMPO ENTRE t_inicial E O INSTANTE ATUAL
    //
	// fazer:
	// - obter o tempo atual com clock_gettime
	// - chamar tempo_diferenca
    return so_tempo_ate_agora();
    //==============================================
}

void tempo_registar(struct timespec *t) {
    //==============================================
    // REGISTAR O TEMPO ATUAL EM t (CLOCK_REALTIME)
    //
	// usar clock_gettime com CLOCK_REALTIME
    so_tempo_registar(t);
    //==============================================
}

void tempo_processamento_encomenda() {
	//==============================================
	// ADORMECER POR 1 MILISEGUNDO
	//
	// usar usleep
	so_tempo_processamento_encomenda();
	//==============================================
}
