// | Fiocco Nicolas | Tiziano Jhonny Floriddia | Giorgio Francesco Floridia |
//struttura della memoria condivisa
#include <stdbool.h>
#include <semaphore.h>

struct struttura {
	//variabili
	int cnt;
	int alimentatore ;			//serve per il controllo dell'alimentatore prima di start e per terminazione
	int attivatore ;			//serve per il controllo dell'attivatore prima di start e per trminazione 
	int nConsumata; 			//energia prelevata da master

	//variabili per gli atomi
	int energiaLiberata;
	int scorie;
	int attivazioni;				//regola la scissione	
	int num_scissioni;

	//variabili da stampare in "tempo reale"
	int us_enLiberata;        
	int us_scorie;
	int us_num_scissioni;
	int tot_enConsumata;
	int us_attivazioni;


	//variabili errori
	int error_meltdown ;
	int error_timeout ;			
	int error_explode ;
	int error_blackout;

	//semafori
	sem_t sem_us_attivazioni;
	sem_t semaforo_cnt_atomo ;
	sem_t semaforo_start ;
	sem_t sem_enLiberata;
	sem_t sem_scorie;
	sem_t sem_error_explode;
	sem_t sem_error_meltdown;
	sem_t sem_error_blackout;
	sem_t sem_attivazioni;
	sem_t sem_attivatore;
	sem_t sem_alimenatore;
	sem_t sem_numScissioni;				
	sem_t sem_us_num_scissioni;
	sem_t sem_us_enLiberata;
	sem_t sem_us_scorie;
	sem_t sem_tot_enConsumata;
	
	
};
