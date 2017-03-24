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

void tempo_iniciar(long intervalo) {
	//==============================================
	// INICIAR ESTRUTURA t_inicial COM VALOR DE RELOGIO (CLOCK_REALTIME)
	//
	so_tempo_iniciar(intervalo);
	//==============================================
}

void tempo_armar_alarme() {
	//==============================================
	// ARMAR ALARME DE ACORDO COM intervalo_alarme (SIGNAL E SETTIMER)
	//
	so_tempo_armar_alarme();
	//==============================================
}

void tempo_escrever_log_temporizado(int signum) {
	//==============================================
	// ESCREVER LOG NO ECRAN DE FORMA TEMPORIZADA
	//
	so_tempo_escrever_log_temporizado(signum);
	//==============================================
}

double tempo_diferenca(struct timespec t1, struct timespec t2) {
	//==============================================
	// CALCULAR A DIFERENCA, EM NANOSEGUNDOS, ENTRE t1 E t2
	//
	return so_tempo_diferenca(t1, t2);
	//==============================================
}

double tempo_ate_agora() {
	//==============================================
	// CALCULAR O INTERVALO DE TEMPO ENTRE t_inicial E O INSTANTE ATUAL
	//
	return so_tempo_ate_agora();
	//==============================================
}

void tempo_registar(struct timespec *t) {
	//==============================================
	// REGISTAR O TEMPO ATUAL EM t (CLOCK_REALTIME)
	//
	so_tempo_registar(t);
	//==============================================
}

void tempo_processamento_encomenda() {
	//==============================================
	// ADORMECER POR 1 MILISEGUNDO
	//
	so_tempo_processamento_encomenda();
	//==============================================
}
