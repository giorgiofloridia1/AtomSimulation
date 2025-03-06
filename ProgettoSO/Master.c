// | Fiocco Nicolas | Tiziano Jhonny Floriddia | Giorgio Francesco Floridia |
#define _GNU_SOURCE 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <semaphore.h>
#include <signal.h>
#include <time.h>
#include "struttura.h"
#include "costanti.h"

#define N_ATOMI_INIT 25

struct struttura * memoria;
int id_memoria;


void azzeramento(){
    sem_wait(&memoria->sem_us_enLiberata);
	memoria->us_enLiberata = 0;
	sem_post(&memoria->sem_us_enLiberata);
    
	sem_wait(&memoria->sem_us_scorie);
	memoria->us_scorie = 0;
    sem_post(&memoria->sem_us_scorie);

    sem_wait(&memoria->sem_us_num_scissioni);
	memoria->us_num_scissioni = 0;
	sem_post(&memoria->sem_us_num_scissioni);
	
	sem_wait(&memoria->sem_us_attivazioni);
	memoria->us_attivazioni = 0;
	sem_post(&memoria->sem_us_attivazioni);
}


void terminazione(int type){	
	alarm(0);					//disabilito timer, simulazione terminata in altro modo										
	while(memoria->cnt>0 && memoria->alimentatore==1 && memoria->attivatore==1){ //aspettare che terminino tutti i processi in modo che tutti abbiano fatto il Detach dalla memoria
			usleep(1000); //0.001 sec
		}
		sem_destroy(&memoria->sem_enLiberata);
		sem_destroy(&memoria->sem_error_blackout);
		sem_destroy(&memoria->sem_error_explode);
		sem_destroy(&memoria->sem_error_meltdown);
		sem_destroy(&memoria->sem_scorie);
		sem_destroy(&memoria->semaforo_cnt_atomo);
		sem_destroy(&memoria->semaforo_start);
		sem_destroy(&memoria->sem_attivazioni);
		sem_destroy(&memoria->sem_alimenatore);
		sem_destroy(&memoria->sem_attivatore);
		sem_destroy(&memoria->sem_tot_enConsumata);
		sem_destroy(&memoria->sem_us_enLiberata);
		sem_destroy(&memoria->sem_us_num_scissioni);
		sem_destroy(&memoria->sem_us_scorie);
		sem_destroy(&memoria->sem_numScissioni);
		sem_destroy(&memoria->sem_us_attivazioni);
		
		
		shmdt(memoria); 	//Detach: distaccamento dalla memoria, indica a SO che il processo non intende piu accedere a quella regione di memoria
		shmctl(id_memoria,IPC_RMID, NULL);	//deallocazione vera e propria della memoria
		fflush(stdout);
		printf("Programma chiuso correttamente\n");
		exit(type);
}


void gestisci_alarm(){   		//signum rappresenta il numero di segnale di allarme che cattura per questa gestione. in questo caso catturerà SIGALR  
	memoria->error_timeout = 1 ;
	perror("Limite di tempo scaduto\n");
	terminazione(0);
}


int main() {
	
	
	//crea l'id dell'area di memoria condivisa, questo id deve essere passato a tutti gli altri processi in modo che riescano ad accedere all'area di memoria condivisa
	id_memoria = shmget(IPC_PRIVATE, sizeof(*memoria), 0600);	//parametro 0600 permette accesso a quella memoria solo all'utente proprietario

	if(id_memoria==-1){                   //gestione nella creazione della memoria condivisa
		perror("Creazione della memoria condivisa fallita\n");
		exit(EXIT_FAILURE);
	}

	//creo una stringa con all'interno l'id, lo devo passare come stringa ai figli
	char stringa_id[8] ;	
	sprintf(stringa_id, "%d", id_memoria);

	//la variabile memoria viene usata per leggere e scrivere sull'area di memoria condivisa
	memoria = shmat(id_memoria, NULL, 0);
		
	//scrittura della memoria condivisa
	memoria->cnt = 0;
	memoria->error_meltdown = 0 ;
	memoria->error_timeout = 0 ;
	memoria->error_blackout=0;
	memoria->error_explode=0;			
	memoria->tot_enConsumata = 0 ;

		
	
	char *arg_Ptr[5];
	arg_Ptr[0] = "figli.c" ;
	arg_Ptr[1] = "0";	//id della memoria condivisa
	arg_Ptr[2] = "0" ;			//attivazioni
	arg_Ptr[4]= NULL;

	//inizializzazione dei semafori
	sem_init(&memoria->semaforo_cnt_atomo, 1, 0) ;	//il secondo indica se è condiviso tra processi o no, terzo quanti processi possono accedere
	sem_init(&memoria->semaforo_start, 1, 0) ;
	sem_init(&memoria->sem_enLiberata, 1, 1) ;
	sem_init(&memoria->sem_scorie, 1, 1) ;
	sem_init(&memoria->sem_error_blackout, 1, 1);
	sem_init(&memoria->sem_error_explode, 1, 1);
	sem_init(&memoria->sem_error_meltdown, 1, 1);
	sem_init(&memoria->sem_attivazioni, 1, 1);
	sem_init(&memoria->sem_alimenatore, 1, 1);
	sem_init(&memoria->sem_attivatore, 1, 1);
	sem_init(&memoria->sem_tot_enConsumata, 1, 1);
	sem_init(&memoria->sem_us_num_scissioni, 1, 1);
	sem_init(&memoria->sem_us_enLiberata, 1, 1);
	sem_init(&memoria->sem_us_scorie, 1, 1);	
	sem_init(&memoria->sem_numScissioni,1,1);
	sem_init(&memoria->sem_us_attivazioni,1,1);
	

	//creazione del timer
	signal(SIGALRM, gestisci_alarm); 		//SIGALRM e il segnale di allarme mandato, gestisci_alarm colui che gestira il segnale
	alarm(SIM_DURATION);

	srand(time(NULL)+clock());	//generazione numeri casuali utilizzerà il tempo corrente come seme + cicli di clock da avvio del programma così sara sempre diversa
	int vettore_casuale[N_ATOMI_INIT];

	for(int k = 0; k < N_ATOMI_INIT; k++){
		vettore_casuale[k] = rand() % N_ATOM_MAX + 1 ;
		//vettore_casuale[k] =N_ATOM_MAX;
	}

	

	pid_t pid ;
	for(int k = 0; k < N_ATOMI_INIT; k++){
		int pipe_fd4[2];				//creazione pipe Atomo
		if(pipe(pipe_fd4)==-1){
		perror("Errore nella creazione della pipe\n");
		terminazione(1);
		}
		pid = fork();
		if(pid == 0){
			//creazione numero atomico per atomi da creare
			char numero_atomico[8];
			sprintf(numero_atomico, "%d", vettore_casuale[k]) ;	//numero atomico
			arg_Ptr[3] = numero_atomico ;

			//IdMemoria e pipe
			close(pipe_fd4[1]);						//figlio non  puo scrivere
			char pipe_fd_str[10];
        	snprintf(pipe_fd_str, sizeof(pipe_fd_str), "%d", pipe_fd4[0]);	//file descriptor da passare come parametro
			arg_Ptr[1]=pipe_fd_str;

			if(execv("Atomo.bin",arg_Ptr) == -1){	
				sem_wait(&memoria->sem_error_meltdown);
				memoria->error_meltdown = 1 ;
				sem_post(&memoria->sem_error_meltdown);
				perror("Terminazione per meltdown (execv in Atomo)\n");
				terminazione(1);
			}

		}else if(pid < 0){
			sem_wait(&memoria->sem_error_meltdown);
			memoria->error_meltdown = 1 ;
			sem_post(&memoria->sem_error_meltdown);
			perror("Terminazione per meltdown (fork)\n");
			terminazione(1);
		}
		else{
			//padre
			close(pipe_fd4[0]);		//padre non legge
			write(pipe_fd4[1], stringa_id,sizeof(stringa_id));	//scrittura
			close(pipe_fd4[1]);
		}
	}

	char *arg_Ptr2[4];
	arg_Ptr2[0] = "figli.c" ;
	arg_Ptr2[2] = "0" ;					
	arg_Ptr2[3] = NULL ;

	int pipe_fd[2];				//creazione pipe attivatore
	if(pipe(pipe_fd)==-1){
		perror("Errore nella creazione della pipe\n");
		terminazione(1);
	}
	
	//Attivatore
	pid = fork();
	if(pid == 0){
		//pipe e id memoria
		close(pipe_fd[1]);						//figlio non  puo scrivere
		char pipe_fd_str[10];
        snprintf(pipe_fd_str, sizeof(pipe_fd_str), "%d", pipe_fd[0]);	//file descriptor da passare come parametro
		arg_Ptr2[1]=pipe_fd_str;
		if(execv("Attivatore.bin",arg_Ptr2) == -1){	                
			sem_wait(&memoria->sem_error_meltdown);
			memoria->error_meltdown = 1 ;
			sem_post(&memoria->sem_error_meltdown);	
			perror("Terminazione per meltdown (execv in attivatore)\n");	
			terminazione(1);
		}
	
	}else if(pid < 0){
		sem_wait(&memoria->sem_error_meltdown);
		memoria->error_meltdown = 1 ;
		sem_post(&memoria->sem_error_meltdown);
		perror("Terminazione per meltdown (fork)\n");
		terminazione(1);
	}
	else{
		//padre
		close(pipe_fd[0]);		//padre non legge
		write(pipe_fd[1], stringa_id,sizeof(stringa_id));	//scrittura
		close(pipe_fd[1]);
	}
	

	int pipe_fd2[2];				//creazione pipe alimentatore
	if(pipe(pipe_fd2)==-1){
		perror("Errore nella creazione della pipe\n");
		terminazione(1);
	}

	//Alimentatore
	
	pid = fork();
	
	
	if(pid == 0){
	
								//figlio non  puo scrivere
		
		
        char numero_atomico[8];
			sprintf(numero_atomico, "%d", vettore_casuale[10]) ;
			arg_Ptr[3] = numero_atomico ;
		
		char pipe_fd_str[10];
		arg_Ptr2[1]=pipe_fd_str;
		close(pipe_fd2[1]);
        snprintf(pipe_fd_str, sizeof(pipe_fd_str), "%d", pipe_fd2[0]);	//file descriptor da passare come parametro
		if(execv("Alimentatore.bin",arg_Ptr2) == -1){  
			sem_wait(&memoria->sem_error_meltdown);
			memoria->error_meltdown = 1 ;
			sem_post(&memoria->sem_error_meltdown);			
			perror("Terminazione per meltdown (execv in alimentatore)\n");
			terminazione(1);
		}
	
	}else if(pid < 0){
		sem_wait(&memoria->sem_error_meltdown);
		memoria->error_meltdown = 1 ;
		sem_post(&memoria->sem_error_meltdown);
		perror("Terminazione per meltdown (fork)\n");
		terminazione(1);
	}
	else{
	
		//padre
		close(pipe_fd2[0]);		//padre non legge
		write(pipe_fd2[1], stringa_id,sizeof(stringa_id));	//scrittura
		close(pipe_fd2[1]);
	}

	

	//inizio controllo degli atomi, alimentatore e attivatore
	sem_post(&memoria->semaforo_cnt_atomo);
	while(memoria->cnt !=  N_ATOMI_INIT){
		fflush(stdout);
		printf("Attendere, gli atomi si stanno inizializzando: \n");
		sleep(1);
	}

	while(memoria->attivatore != 1){
		fflush(stdout) ;
		printf("Attendere, l'attivatore si sta inizializzando %d\n", memoria->attivatore);
		sleep(1) ;
	}

	while(memoria->alimentatore != 1){
		fflush(stdout) ;
		printf("Attendere, l'alimentatore si sta inizializzando\n") ;
		sleep(1) ;
	}
	
	sem_post(&memoria->semaforo_start);
	
	while (memoria->error_timeout != 1){
		sleep(1);

		//controllo errori
		if(memoria->error_meltdown == 1){
			terminazione(1);
		}
		
		if(memoria->energiaLiberata > ENERGY_EXPLODE_THRESHOLD){
		    sem_wait(&memoria->sem_error_explode);
			memoria->error_explode = 1;
			sem_post(&memoria->sem_error_explode);
			perror("Terminazione per explode\n");
			terminazione(1);
	    }
		
		sem_wait(&memoria->sem_tot_enConsumata);
		memoria->tot_enConsumata += ENERGY_DEMAND ;
        sem_post(&memoria->sem_tot_enConsumata);

			
		//prelievo energia, dopo controllo dato che errore Explode deve avvenire senza tener conto del prelievo di master
		sem_wait(&memoria->sem_enLiberata);
		memoria->energiaLiberata-= ENERGY_DEMAND;
		if(memoria->energiaLiberata<0){
			sem_wait(&memoria->sem_error_blackout);
			memoria->error_blackout=1;
			sem_post(&memoria->sem_error_blackout);
			perror("Terminazione per blackout\n");
			terminazione(1);
		}
		sem_post(&memoria->sem_enLiberata);
		
		fflush(stdout);  // Svuota il buffer di output, in modo che il processo padre possa stampare
		//printf("Atomi: %d ; ", memoria->cnt);
		printf("--------------------------STATISTICHE TOTALI------------------------\n");
		printf("Energia prodotta: %d ; ", memoria->energiaLiberata);
		printf("Energia consumata: %d ; ", memoria->tot_enConsumata);			// usata/prelevata da master
		printf("Scorie: %d ; \n", memoria->scorie);
		printf("Numero Scissioni: %d ; ", memoria->num_scissioni);
		printf("Numero Attivazioni: %d ; \n", memoria->attivazioni);
		printf("-------------------STATISTICHE ULTIMO SECONDO---------------\n");
		printf("Energia prodotta: %d ; ", memoria->us_enLiberata);
		printf("Energia cosumata: %d ; ", ENERGY_DEMAND);
		printf("Scorie: %d ;\n", memoria->us_scorie);
		printf("Numero Scissioni: %d ; ", memoria->us_num_scissioni);
		printf("Numero Attivazioni: %d ;\n", memoria->us_attivazioni);
		
		
		printf("\n");
		
		azzeramento();	
	}

	//atomi finiti, terminare
	fflush(stdout);
	terminazione(0);
}
