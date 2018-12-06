#include <linux/module.h>
#include <asm/io.h>
#include <asm/rtai.h>
#include <rtai_shm.h>
#include <rtai_sched.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/stat.h>
#include "parameters.h"


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Popov");
 
static RT_TASK timers[DIM];
static int arr_argc=0;
static int wcet[DIM];
static int contatori[DIM];
static int periodi[DIM];
static int tempoInizio[DIM]={-1,-1,-1};
static float ulub[5]={1.0,0.828,0.757,0.743,0.735};
static Buffer* b;

int test_sched(void){
	int i;
	float r=0.0;
	for(i=0;i<DIM;i++)
		r+=(float)(wcet[i])/(float)(periodi[i]);
	if(r>ulub[DIM])
	return 0;
	else
	return 1;
	
}

int getMCD(void){
	int i,a,b,c,resto,mcd;
	for(i=0;i<DIM-1;i++){
		a=periodi[i];
		b=periodi[i+1];
		if(b>a){//swap
		c=a;
		a=b;
		b=c;
		}
		while(a%b!=0){
			resto=a%b;
			a=b;
			b=resto;
		}
		if(i==0)
			mcd=b;
		else if(b<mcd)
			mcd=b;
		

		}
	return mcd;
}


module_param_array(wcet, int, &arr_argc, 0000);
MODULE_PARM_DESC(wcet, "WCET: ");

module_param_array(contatori, int, &arr_argc, 0000);
MODULE_PARM_DESC(contatori, "Contatori: ");

module_param_array(periodi, int, &arr_argc, 0000);
MODULE_PARM_DESC(periodi, "Periodi: ");





void task_routine(long index){

	int inizio,fine,newval;
	int lateness=0;
	inizio=count2nano(rt_get_time());

	if(tempoInizio[index]==-1)
		tempoInizio[index]=count2nano(rt_get_time());
	
	while(contatori[index]>=0){
		rt_printk("[TIMER %d] Cont: %d Sys_time: %d \n",index,contatori[index],count2nano(rt_get_time()));
		contatori[index]--;

		if(contatori[index]>=0){

			fine=count2nano(rt_get_time());
			newval=fine-inizio;
			if(newval>wcet[index])
				wcet[index]=newval;
			lateness=count2nano(next_period())-fine;

			if(lateness> L_MAX)
			rt_printk("[TIMER %d] Warning!!! Lateness %d \n",index,lateness);

			rt_task_wait_period();
			}
			if(contatori[index]==0){
			b[index].indice=index;
			b[index].cont=1;
			
			rt_printk("[TIMER %d] SCADUTO. WCET: %d Tempo trascorso: %d \n",index,wcet[index],fine-tempoInizio[index]);
		
			}
	}
	
	
}


int init_module(){
	long i;
    	rt_set_periodic_mode();
	b=rtai_kmalloc(shm_name,sizeof(Buffer)*DIM);
	if(test_sched()==0)
		rt_printk("===== insieme dei task non schedulabile =====");
	else{
		int p=getMCD();	
		start_rt_timer(nano2count(p));
		rt_printk("===== insieme dei task schedulabile con periodo (MCD) %d =====",p);
		for(i=0;i<DIM;i++){
			rt_task_init(&timers[i], task_routine, i, STACK_SIZE, 1, 1, 0);
			b[i].cont=0;
		    	rt_task_make_periodic(&timers[i], rt_get_time()+nano2count(periodi[i]), nano2count(periodi[i]));
		}	
		rt_spv_RMS(hard_cpu_id());
	}
    return 0;

}

void cleanup_module(){
	int i;
    	stop_rt_timer();
	for(i=0;i<DIM;i++)
    	rt_task_delete(&timers[i]);
	rtai_kfree(shm_name);
   

    return;

}




