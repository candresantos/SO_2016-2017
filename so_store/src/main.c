/*Made by: G25
 *Carlos Santos 47805
 *Vasco Teixeira 45156
 *João Serrano 47868
 */
#include <assistente.h>
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
#include <loja.h>

#include "main.h"
#include "cliente.h"
#include "memoria.h"
#include "prodcons.h"
#include "controlo.h"
#include "ficheiro.h"
#include "tempo.h"
#include "so.h"

struct indicadores Ind;     // indicadores do funcionamento do SO_Store
struct configuracao Config; // configuração da execução do SO_Store

long main_args(int, char **, char**, char**, char**);
void print_error();
int check_if_flag(int, char **, int, int, int, int);
void set_things(int, char **,int *, int *, int *, int *,
											char **, char **, long *);
void process_maker(int, int**,int(*f)(int));
void wait_for_process(int, int**, int*, int);

/* main_cliente recebe como parâmetro o nº de clientes a criar */
void main_cliente(int quant) {
	//Create quant processes and store their PID at Ind.pid_clientes
	process_maker(quant, &Ind.pid_clientes, &cliente_executar);
}

/* main_assistente recebe como parâmetro o nº de assistentes a criar */
void main_assistente(int quant) {
	//Create quant processes and store their PID at Ind.pid_assistentes
	process_maker(quant, &Ind.pid_assistentes, &assistente_executar);
}

/* main_loja recebe como parâmetro o nº de lojas a criar */
void main_loja(int quant) {
	//Create quant processes and store their PID at Ind.pid_lojas
	process_maker(quant, &Ind.pid_lojas, &loja_executar);
}

int main(int argc, char* argv[]) {
	int n, result;
	char *ficEntrada = NULL;
	char *ficSaida = NULL;
	char *ficLog = NULL;
	long intervalo = 0;

	intervalo = main_args(argc, argv, &ficEntrada, &ficSaida, &ficLog);
	printf(
			"\n------------------------------------------------------------------------");
	printf(
			"\n-------------------------- Aplicacao SO_Store --------------------------");
	printf(
			"\n------------------------------------------------------------------------\n");

	// Ler dados de entrada
	ficheiro_iniciar(ficEntrada, ficSaida, ficLog);

	// criar zonas de memória e semáforos
	memoria_criar_buffers();
	prodcons_criar_buffers();
	controlo_criar();

	// Criar estruturas para indicadores e guardar stock inicial
	memoria_criar_indicadores();
	printf("\n\t\t\t*** Abrir SO_Store ***\n\n");
	controlo_abrir_sostore();

	// Registar início de operação e armar alarme
	tempo_iniciar(intervalo);

	//
	// Iniciar sistema
	//
	// Criar ASSISTENTES
	main_assistente(Config.ASSISTENTES);
	// Criar LOJAS
	main_loja(Config.LOJAS);
	// Criar CLIENTES
	main_cliente(Config.CLIENTES);

	// ESPERAR PELA TERMINAÇÃO DOS CLIENTES E ATUALIZAR INDICADORES
	wait_for_process(Config.CLIENTES, 
			&Ind.produtos_obtidos_pelos_clientes, Ind.pid_clientes, 0);

	printf("\n\t\t\t*** Fechar SO_Store ***\n\n");
	controlo_fechar_sostore();

	// ESPERAR PELA TERMINAÇÃO DOS CLIENTES E ATUALIZAR INDICADORES
	wait_for_process(Config.ASSISTENTES,
			&Ind.clientes_atendidos_pelas_assistentes, 
											Ind.pid_assistentes, 1);

	// ESPERAR PELA TERMINAÇÃO DOS CLIENTES E ATUALIZAR INDICADORES
	wait_for_process(Config.LOJAS,
			&Ind.clientes_atendidos_pelas_lojas, Ind.pid_lojas, 1);

	printf(
			"------------------------------------------------------------------------\n\n");
	printf("\t\t\t*** Indicadores ***\n\n");
	so_escreve_indicadores();
	printf("\t\t\t*******************\n");

	// destruir zonas de memória e semáforos
	ficheiro_destruir();
	controlo_destruir();
	prodcons_destruir();
	memoria_destruir();

	return 0;
}

/*
 * Receives an int describing how many arguments shell receives,
 * a copy of the array of the arguments,
 * a pointer to ficEntrada, ficSaida, ficLog
 * Returns the interval set by the user (using the -t flag) or the
 * default time (0)
 */
long main_args(int argc, char* argv[], char **ficEntrada,
									char **ficSaida, char **ficLog){
	//Iteration, log flag not set, time flag not set, output not set
	int i, l = 0, t = 0, o = 0;
	long interval = 0;
	//Missing input file
	if(argc < 2){
		//Print how to use and exit
		print_error(0);
	}
	for(i = 1; i < argc; i++){
		//Expected input file name
		if(i == 1){
			//If it detects a flag prints how to use and exits
			if((strcmp(argv[i],"-l") == 0) || 
									(strcmp(argv[i],"-t") == 0))
				print_error(0);
			//File name passed onto ficEntrada
			*ficEntrada = argv[i];
		} else
			//Changes what it has to change depending on the argument
			set_things(argc, argv, &i, &l, &t, &o,
								&(*ficLog), &(*ficSaida), &interval);
	}
	return interval;
}

/*
 * Receives an int describing how many arguments shell receives,
 * a copy of the arguments, the current argument, and the flags.
 * It also receives the log file pointer, the output file pointer and
 * the interval.
 * Set's everything up, changing the current argument, the flags
 * (if received the argument to set it) and the corresponding variable.
 */
void set_things(int argc, char **argv,
					int * i, int * l, int * t, int * o,
					 char **ficLog, char ** ficSaida, long *interval){
	int flag;
	//It's flag -l
	if((flag = check_if_flag(argc, argv, *i, *l, *t, *o)) == 1){
		//Change i and gives a value to ficLog
		*ficLog = argv[++*i];
		//Set flag
		*l = 1;
	}
	else if (flag){
		//Change i and gives a value to interval
		*interval = atol(argv[++*i]);
		//Set flag
		*t = 1;
	}
	else{
		//Gives value to ficSaida
		*ficSaida = argv[*i];
		//Set flag
		*o = 1;
	}
}

/*
 * Receives an int describing how many arguments shell receives,
 * a copy of the arguments, the current argument, and the flags.
 * Returns 0 if it's the output, 1 if it's the -l flag, or 2 if it's
 * the -t flag
 */
int check_if_flag(int argc, char **argv, int i, int l, int t, int o){
	int result = 0;
	//If it's -l flag
	if(strcmp(argv[i],"-l") == 0){
		//If already set
		if(l)
			print_error(4);
		//If it doesn't have any more arguments prints how to use and exits 
		if(argc < i+2)
			print_error(3);
		result = 1;
	}
	//If it's -t flag
	else if (strcmp(argv[i],"-t") == 0){
		//If already set
		if(t)
			print_error(4);
		//If it doesn't have any more arguments
		if(argc < i+2)
			print_error(3);
		//If it's not a number
		if(!strtol(argv[i+1],NULL,0))
			print_error(5);
		result = 2;
	//Output
	} else {
		//If already set
		if(o)
			print_error(4);
	}
	
	return result;
}

/*
 * Receives an integer describing an error.
 * 0 -> No input file
 * 1 -> Extra -l flag
 * 2 -> Extra -t flag
 * 3 -> No parameter after flag
 * Prints the error and message how to use
 */
void print_error(int error){
	char error_message[] = "Como usar: sostore ficheiro_configuracao \
[ficheiro_resultados] -l [ficheiro_log] -t [intervalo(us)]\n\
Exemplo: ./bin/sostore testes/in/cenario1 \
testes/out/cenario1 -l testes/log/cenario1.log -t 1000\n";
	switch (error){
		case 0:
			printf("Falta o ficheiro de input!\n%s",error_message);
			break;
		case 1:
			printf("-l já usado!\n%s",error_message);
			break;
		case 2:
			printf("-t já usado!\n%s",error_message);
			break;
		case 3:
			printf("Falta o parametro depois da flag!\n%s",
														error_message);
			break;
		case 4:
			printf("Flag já foi usada!\n%s",error_message);
			break;
		default:
			printf("%s",error_message);
			break;
	}
	exit(EXIT_FAILURE);
}

/*
 * Receives how many processes to create, the pointer to the memory
 * to store PIDs and a pointer to a function to apply.
 */
void process_maker(int quant, int **pid_pointer, int (*f)(int)){
	//Child PID
	pid_t pid;
	//Iteration and exit value
	int i, prod;
	for(i = 0; i < quant; i++){
		//Fork failed
		if((pid = fork())==-1){
			perror("Fork failed");
			exit(EXIT_FAILURE);
		}
		//Code executed by child process
		else if(!pid){
			//Call function f
			prod = (*f)(i);
			exit(prod);
		}
		//Code executed by parent process
		else{
			//Store the PID of the child process
			(*pid_pointer)[i] = pid;
		}
	}
}

/*
 * Receives the size, a pointer to the memory to store the exit
 * value of the processes, a copy of the Child process PIDs,
 * and if it's to fill the second argument by order.
 * 0 -> The exit status is the location to increment
 * 1 -> The exit status is the actual value 
 */
void wait_for_process(int quant, int **ret_mem_pointer, 
										 int *pid_pointer, int order){
	//Interation and return value
	int i, ret;
	for(i = 0; i < quant; i++){
		/*Wait for the ith PID in pid_pointer, storing the exit status
		 in ret*/
		waitpid(pid_pointer[i],&ret,0);
		//If it exited using exit or _exit
		if(WIFEXITED(ret)){
			//The value of the exit/_exit
			int value = WEXITSTATUS(ret);
			//Value is the value of the ith position in (*ret_mem_pointer)
			if(order)
				(*ret_mem_pointer)[i] = value;
			//If it was in stock, increment the valueth position
			else if(value != Config.PRODUTOS){
				(*ret_mem_pointer)[value]++;
			}
		}
	}
}
