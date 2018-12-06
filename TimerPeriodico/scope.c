
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include "/usr/realtime/include/rtai_shm.h"
#include "parameters.h"




 

int main (void)

{
	Buffer* b;
	int i=0;
	int cnt=DIM;

      	b=rtai_malloc(shm_name,1);
	
	while(cnt>0){
	
	
	if(b[i].cont==1){
	
	 
	 printf("[TASK %d] Scaduto !!! \n", b[i].indice);
	 b[i].cont=0;
	 cnt--;
	}
	i=++i%DIM;
    }

    rtai_free (shm_name, &b);

    
    return 0;

}
