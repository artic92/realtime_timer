#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <rtai_nam2num.h>
#include <rtai_shm.h>
#include <rtai_lxrt.h>
#include "parameters.h"

int main(void){
	//Attachment alla struttura di comunicazione con il kernel thread. NB: la deallocazione viene effettuata dal kernel thread.
	struct info* i = (struct info*)(rtai_malloc(nam2num("SHMNAM"), sizeof(struct info)));
	int task_running = i->num_task, j, k;
	
	//Vettore di variabili di stato, utilizzato per mantenere traccia dei timer segnalati.
	unsigned int visualizzato[NUMTSK];
	for(k = 0; k < NUMTSK; k++)
		visualizzato[k] = 0;
	
	printf("[Buddy Task (PID: %d)]Mi metto in attesa della terminazione dei timer.\n", getpid());
	
	while(task_running != 0){
		for(j = 0; j < NUMTSK; j++){
			if(i->terminato[j] == 1 && visualizzato[j] == 0){
				printf("[Buddy Task (PID: %d)]Timer %u ha terminato!\n", getpid(), j);
				visualizzato[j] = 1;
				task_running--;
				}
		}
	}
	return 0;
}
