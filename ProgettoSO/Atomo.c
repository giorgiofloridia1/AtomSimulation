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
#include <stdbool.h>
#include <time.h>
#include "struttura.h"
#include "costanti.h"



struct struttura * memoria;

int energiaLiberata(int a, int b){		
    int max = a>=b? a : b;
	int totale = a*b - max;
	return totale;
}

void terminazione(int type){
	sem_wait(&memoria->semaforo_cnt_atomo);
	memoria->cnt-=1;
	sem_post(&memoria->semaforo_cnt_atomo);
	shmdt(memoria); 	//Detach: distaccamento dalla memoria, indica a SO che il processo non intende piu accedere a quella regione di memoria
	exit(type);
}


int main(int argc, char *argv[]) {
	int attivazioni = atoi(argv[2]);
	
	int numAtomico = atoi(argv[3]);
	int numAtomicoFiglio = numAtomico / 2;//creazione num atomico da passare al figlio(nel caso poi verrà scisso
	

	 //recevuto messaggio id memoria la si memorizza e usa
	int pipe_fd=atoi(argv[1]);
	char mem_str[10];
	read(pipe_fd, mem_str,sizeof(mem_str));
	int id_memoria= atoi(mem_str);
	close(pipe_fd);						//chiusa lettura

	//collegamento tra la variabile memoria e la memoria condivisa
	memoria = shmat(id_memoria, NULL, 0);

	//verifica tutti atomi inizializzati
    sem_wait(&memoria->semaforo_cnt_atomo);
    memoria->cnt = memoria->cnt + 1 ;
	sem_post(&memoria->semaforo_cnt_atomo);

	//il semaforo serve per far partire l'atomo dopo che tutto è stato inizializzato
	sem_wait(&memoria->semaforo_start);
	sem_post(&memoria->semaforo_start);

	while(true){
	
		sem_wait(&memoria->sem_attivazioni);
		memoria->attivazioni = memoria->attivazioni + 1 ;
		sem_post(&memoria->sem_attivazioni);
		sem_wait(&memoria->sem_us_attivazioni);
		memoria->us_attivazioni = memoria->us_attivazioni+1;
		sem_post(&memoria->sem_us_attivazioni);
		
	
	
		while(memoria->attivazioni != attivazioni){
			//controllo errori
			if(memoria->attivazioni>attivazioni){
				attivazioni=memoria->attivazioni;
			}

			if(memoria->error_meltdown == 1){		
				terminazione(1);
			}

			if(memoria->error_explode == 1){	
				terminazione(1);
			}

			if(memoria->error_blackout==1){
				terminazione(1);
			}
			if(memoria->error_timeout == 1){	
				terminazione(1);
			}

			usleep(1000);                     
		}
			

			
			
	
		
		if(numAtomico > MIN_N_ATOMICO && memoria->attivazioni == attivazioni){			//deve scindere
			sem_wait(&memoria->sem_us_num_scissioni);
            memoria->us_num_scissioni++;
			sem_post(&memoria->sem_us_num_scissioni);
			sem_wait(&memoria->sem_numScissioni);
			memoria->num_scissioni++;
			sem_post(&memoria->sem_numScissioni);
			char attivazioniDaPassare[8];
			sprintf(attivazioniDaPassare,"%d",attivazioni);
			
			//argomento da mandare ai figli con informazioni all'interno(id della memoria condivisa)
			char numAtomDaPassare[8];									//per passare il num atomico come array di char
			sprintf(numAtomDaPassare, "%d", numAtomicoFiglio);			//valorizzazzione dell array di char per poi passarlo come parametro arg_Ptr[3f]
			
			char *arg_Ptr[5];
			arg_Ptr[0] = "figli.c";
			arg_Ptr[1] = "0" ;						//file descriptor pipe
			arg_Ptr[2] = attivazioniDaPassare; 		//attivazioni
			arg_Ptr[3] = numAtomDaPassare;
			arg_Ptr[4]= NULL;
			
			//creazione pipe
			int pipe_fd[2];				
			if(pipe(pipe_fd)==-1){
				perror("Errore nella creazione della pipe in Atomo\n");
				terminazione(1);
			}

			pid_t pid= fork();                                    //creazione processo figlio	
			if(pid == 0){
				//IdMemoria e pipe
				close(pipe_fd[1]);						//figlio non  puo scrivere
				char pipe_fd_str[10];
        		snprintf(pipe_fd_str, sizeof(pipe_fd_str), "%d", pipe_fd[0]);	//file descriptor da passare come parametro
				arg_Ptr[1]=pipe_fd_str;

				if(execv("Atomo.bin",arg_Ptr)==-1){			//l'if esegue execv		
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
				close(pipe_fd[0]);		//padre non legge
				char stringa_id[8] ;	
				sprintf(stringa_id, "%d", id_memoria);
				write(pipe_fd[1], stringa_id,sizeof(stringa_id));	//scrittura
				close(pipe_fd[1]);
			}	

			//codice processo padre
			int enLiberata= energiaLiberata(numAtomico, numAtomicoFiglio);
			
			
			sem_wait(&memoria->sem_enLiberata);
			memoria->energiaLiberata+=enLiberata;						
			sem_post(&memoria->sem_enLiberata);
			sem_wait(&memoria->sem_us_enLiberata);
			memoria->us_enLiberata += enLiberata;	
			sem_post(&memoria->sem_us_enLiberata);


			

			numAtomico=numAtomico-numAtomicoFiglio;	// trasfromazione num atomico padre
			

		}else if(numAtomico <= MIN_N_ATOMICO ){						//Scissione non deve avvenire, atomo in scorie, 
				sem_wait(&memoria->sem_scorie);
				memoria->scorie++;
				sem_post(&memoria->sem_scorie);

				sem_wait(&memoria->sem_us_scorie);
				memoria->us_scorie++;
				sem_post(&memoria->sem_us_scorie);
				terminazione(0);
			}
			
	}
}
