// | Fiocco Nicolas | Tiziano Jhonny Floriddia | Giorgio Francesco Floridia |
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <time.h>
#include "struttura.h"
#include "costanti.h"
#define N_NUOVI_ATOMI 10

struct struttura * memoria;                    //collegamento alla memoria

void terminazione(int type){                
    memoria->alimentatore = 0;
    shmdt(memoria);     //Detach: distaccamento dalla memoria, indica a SO che il processo non intende piu accedere a quella regione di memoria
	exit(type);
}

int main(int argc, char *argv[]){
    
    //recevuto messaggio id memoria la si memorizza e usa
	int pipe_fd=atoi(argv[1]);
	char mem_str[10];
	read(pipe_fd, mem_str,sizeof(mem_str));
	int id_memoria= atoi(mem_str);
	close(pipe_fd);
	
    char stringa_id[8] ;	
	sprintf(stringa_id, "%d", id_memoria);
						//chiusa lettura
	if(id_memoria == 0){
		perror("Errore: id_memoria non conseguita in Alimentatore");
		terminazione(1);
	}

    memoria = shmat(id_memoria, NULL, 0);         //"collegamento" tra la variabile memoria e la memoria condivisa

    sem_wait(&memoria->sem_alimenatore);
    memoria->alimentatore = 1;
    sem_post(&memoria->sem_alimenatore);

    

    char *arg_Ptr[5];
	arg_Ptr[0] = "figli.c" ;
	arg_Ptr[1] = mem_str;	//id della memoria condivisa
	arg_Ptr[2] = "0" ;			//attivazioni
    arg_Ptr[4]= NULL;
    
    //semaforo per iniziare
    
    sem_wait(&memoria->semaforo_start);
sem_post(&memoria->semaforo_start);
    
	
   
    //struct per nanosleep
    struct timespec interval;
    interval.tv_sec=0;  //numero di secondi
    interval.tv_nsec=STEP; //numero di nanosecondi

    

    while(memoria->error_timeout != 1){

    
    	pid_t pid ;
        int vettore_casuale[N_NUOVI_ATOMI];
		for(int k = 0; k < N_NUOVI_ATOMI; k++){
		vettore_casuale[k] = rand() % N_ATOM_MAX + 1 ;
	}
        for(int i=0;i<N_NUOVI_ATOMI;i++){
        
        	int pipe_fd4[2];//creazione pipe Atomo
		if(pipe(pipe_fd4)==-1){
		perror("Errore nella creazione della pipe\n");
		terminazione(1);
		}
		pid = fork();
		
		if(pid == 0){
			//creazione numero atomico per atomi da creare
			char numero_atomico[8];
			sprintf(numero_atomico, "%d", vettore_casuale[i]) ;//numero atomico
			arg_Ptr[3] = numero_atomico ;

			//IdMemoria e pipe
			close(pipe_fd4[1]);	//figlio non  puo scrivere
			char pipe_fd_str[10];
        	snprintf(pipe_fd_str, sizeof(pipe_fd_str), "%d", pipe_fd4[0]);	//file descriptor da passare come parametro
			arg_Ptr[1]=pipe_fd_str;

			if(execv("Atomo.bin",arg_Ptr) == -1){	
				sem_wait(&memoria->sem_error_meltdown);
				memoria->error_meltdown = 1 ;
				sem_post(&memoria->sem_error_meltdown);
				perror("Terminazione per meltdown (execv in Master)\n");
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
    
    
    
    
    
       nanosleep(&interval,NULL);
       
       //controllo errori
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
        
    }
    terminazione(0);
}
