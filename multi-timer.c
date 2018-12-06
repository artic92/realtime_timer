#include <linux/moduleparam.h>
#include <asm/io.h>
#include <asm/rtai.h>
#include <rtai_shm.h>
#include <rtai_sched.h>
#include <rtai_nam2num.h>
#include "parameters.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Antonio Riccio, Stefano Sorrentino");
MODULE_PARM_DESC(WCET, "An estimate of Worst-Case Execution Time (assumed equal for all tasks) in ns");
MODULE_PARM_DESC(conteggi, "An array of counting variables for each task");
MODULE_PARM_DESC(periodi, "An array of period for each task in ms");

static unsigned int WCET = 1;
static unsigned int conteggi[NUMTSK] = { 1000, 1000, 1000, 1000, 1000 };
static unsigned int periodi[NUMTSK] = { 10, 10, 10, 10, 10 };
static int array_c_argc = 0;
static int array_p_argc = 0;
static float Ulub[5] = {1, 0.828, 0.780, 0.757, 0.743};
static RT_TASK rt_task[NUMTSK];
static struct info* i;

module_param(WCET, int, 0000);
module_param_array(conteggi, int, &array_c_argc, 0000);
module_param_array(periodi, int, &array_p_argc, 0000);

//Funzioni di servizio per la conversione dei valori da ms a ns e viceversa
static unsigned int ms2ns(unsigned int value){return (unsigned int)(value*1000000);}
static unsigned int ns2ms(unsigned int value){return (unsigned int)(value/1000000);}

static void routine(long t){
	RTIME arrival_time_ns = rt_get_time_ns(), finish_time_ns, total_time_ns = arrival_time_ns, arrival_time_i_ns, lateness_ns = 0, WCET_i_ns = WCET;
	unsigned int counter = conteggi[t];
    	while (counter != 0) {
    		arrival_time_i_ns = rt_get_time_ns();
        	rt_printk("Timer_%u[%u] = ", t, rt_get_time_ns());
        	rt_printk("%u\n", counter);
        	counter--;
        	//Calcolo Ci
        	finish_time_ns = rt_get_time_ns() - arrival_time_i_ns;
		//Calcolo WCET del i-esimo timer
    		if(finish_time_ns > WCET_i_ns)
    			WCET_i_ns = finish_time_ns;
        	//Calcolo Lateness. NB: si assume che la deadline assoluta sia uguale al periodo del timer.
        	lateness_ns = finish_time_ns - ms2ns(periodi[t]);
        	if(lateness_ns > ms2ns(-1))
        		rt_printk("ATTENZIONE: Lateness superiore a -1 ns (%d ns)!!\n", lateness_ns);
        	rt_task_wait_period();
    	}
    	total_time_ns = rt_get_time_ns() - arrival_time_ns;
    	rt_printk("\n");
    	rt_printk("Timer_%u SCADUTO!!\n", t);
    	rt_printk("-- WCET: %u ns\n", WCET_i_ns);
    	rt_printk("-- Tempo trascorso: %u ns (", total_time_ns);
    	rt_printk("%d ms)\n\n", ns2ms(total_time_ns));
    	//Avviso al buddy task del termine del conteggio
    	i->terminato[t] = 1;
}

static int test_sched(void){
	int j;
	float sommatoria = 0;
	//------------Test di Liu e Leiland--------------
	for(j = 0; j < array_p_argc; j++){
		sommatoria += (float)(WCET/(ms2ns(periodi[j])));
	}
    	if(sommatoria > Ulub[array_c_argc-1])
    		return -1;
    	else
    		return 0;
}

int init_module(void){
	int j;
	//Test di schedulabilità
    	if(test_sched() < 0){
    		rt_printk("L'insieme di task non è schedulabile!!\n\n");
    		return -1;
    	}
    	//Inizializzazione struttura di comunicazione con il buddy task
	i = (struct info*)rtai_kmalloc(nam2num("SHMNAM"), sizeof(struct info));
	i->num_task = array_c_argc;
	for(j = 0; j < array_c_argc; j++)
		i->terminato[j] = 0;
	//Settaggio del timer
	rt_set_oneshot_mode();
	start_rt_timer(0);
	//Inizializzazione task periodici. NB: ogni task parte distanziato del proprio periodo.
	for(j = 0; j < array_c_argc; j++){
		rt_task_init(&rt_task[j], routine, j, STACK_SIZE, TASK_PRIORITY, 1, 0);
		rt_task_make_periodic(&rt_task[j], rt_get_time() + nano2count(ms2ns(periodi[j])), nano2count(ms2ns(periodi[j])));
	}
	// ordinamento secondo Rate Monotonic
	rt_spv_RMS(hard_cpu_id());
	return 0;
}

void cleanup_module(void){
	int j;
	rtai_kfree(nam2num("SHMNAM"));
    	stop_rt_timer();
    	for(j = 0; j < array_c_argc; j++){
    		rt_task_delete(&rt_task[j]);
    	}
    	return;
}
