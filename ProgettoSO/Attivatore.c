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
#include "struttura.h"
#include "costanti.h"

struct struttura * memoria;

void terminazione(int type){
	sem_wait(&memoria->sem_attivatore);
    memoria->attivatore = 0;
    sem_post(&memoria->sem_attivatore);
	shmdt(memoria);     //Detach: distaccamento dalla memoria, indica a SO che il processo non intende piu accedere a quella regione di memoria
	exit(type);
}

int main(int argc, char *argv[]){
    // collegamento alla memoria
	int pipe_fd=atoi(argv[1]);
	char mem_str[10];
	read(pipe_fd, mem_str,sizeof(mem_str));
	int id_memoria= atoi(mem_str);
	close(pipe_fd);						//chiusa lettura
	if(id_memoria == 0){
		perror("Errore: id_memoria non conseguita in Attivatore\n");
		exit(EXIT_FAILURE);
	}
	
	//"collegamento" tra la variabile memoria e la memoria condivisa
	memoria = shmat(id_memoria, NULL, 0);
	memoria->attivatore = 1 ;
	memoria->attivazioni = 0 ;

	//il semaforo serve per far partire l'attivatore dopo che tutto è stato inizializzato
	sem_wait(&memoria->semaforo_start);
	sem_post(&memoria->semaforo_start);
	
	
	while(memoria->cnt != 0){
	
	sem_wait(&memoria->sem_attivazioni);
	memoria->attivazioni = memoria->attivazioni + 1 ; 
	sem_post(&memoria->sem_attivazioni);
	 
	usleep(500000);

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
    
	
	//atomi finiti, terminare processo
	terminazione(0);
}
