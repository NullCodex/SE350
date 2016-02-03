/**
 * @file:   k_process.c  
 * @brief:  process management C file
 * @author: Yiqing Huang
 * @author: Thomas Reidemeister
 * @date:   2014/02/28
 * NOTE: The example code shows one way of implementing context switching.
 *       The code only has minimal sanity check. There is no stack overflow check.
 *       The implementation assumes only two simple user processes and NO HARDWARE INTERRUPTS. 
 *       The purpose is to show how context switch could be done under stated assumptions. 
 *       These assumptions are not true in the required RTX Project!!!
 *       If you decide to use this piece of code, you need to understand the assumptions and
 *       the limitations. 
 */

#include <LPC17xx.h>
#include <system_LPC17xx.h>
#include "uart_polling.h"
#include "k_process.h"

#ifdef DEBUG_0
#include "printf.h"
#endif /* DEBUG_0 */

/* ----- Global Variables ----- */
PCB **gp_pcbs;                  /* array of pcbs */
PCB *gp_current_process = NULL; /* always point to the current RUN process */

U32 g_switch_flag = 0;          /* whether to continue to run the process before the UART receive interrupt */
                                /* 1 means to switch to another process, 0 means to continue the current process */
				/* this value will be set by UART handler */
				  
/*	Represents two queues - 
*		one for ready processes
*		one for blocked processes  
*/
PCB *headBlocked = NULL;
PCB *tailBlocked = NULL;
PCB *headReady = NULL;
PCB *tailReady = NULL;

/* process initialization table */
PROC_INIT g_proc_table[NUM_TEST_PROCS];
extern PROC_INIT g_test_procs[NUM_TEST_PROCS];

/**
*	Null Process
*
*/
void null_process() {
	while (1) {
		k_release_processor () ;
	}
}

PCB* rpq_dequeue(void) {
		PCB* temp;
		if(headReady) {
			temp = headReady;
			headReady = headReady->next;
			return temp;
		}
		return NULL;
}

void rpq_enqueue (PCB *current_process) {
	PCB* temp = headReady;
	PCB* prev = NULL;
	
	if (headReady == NULL) {
		headReady = tailReady = current_process;
	} else {
		if (headReady == tailReady) {
			if (headReady->m_priority < current_process->m_priority) {
				headReady->next = current_process;
				tailReady = current_process;
			} else {
				current_process->next = tailReady;
				headReady = current_process;
				tailReady = current_process->next;
			}
		} else {
			while (temp != tailReady->next) {
				if (temp->m_priority < current_process->m_priority) {
					prev = temp;
					temp = temp->next;
				} else {
					if (headReady == temp) {
						headReady = current_process;
					}
					current_process->next = temp;
					prev->next = current_process;
					break;
				}
			}
		}
	}
}

/*
void rpq_enqueue(PCB *current_process) {
	PCB* temp = headReady;
	PCB* prev = headReady;
	// ready queue is empty
	if(headReady == NULL) {
		headReady = current_process;
		tailReady = current_process;
	} 
	else {
		if(current_process->m_priority < headReady->m_priority) 
		{
			current_process->next = headReady;
			headReady = current_process;
		} 
		else {
			while(current_process->m_priority > temp->m_priority)
            {
                if(temp->next == NULL) {
									break;
								}
                    
								prev = temp;
                temp = temp->next;
            }
			
			//New node id's smallest than all others
            if(temp->next == NULL && current_process->m_priority > temp->m_priority)
            {
                tailReady->next = current_process;
                tailReady = current_process;
            }
            else//New node id's is in the medium range.
            {
                prev->next = current_process;
                current_process->next = temp;
            }
		}
	}	
}
*/
/**
 * @biref: initialize all processes in the system
 * NOTE: We assume there are only two user processes in the system in this example.
 */
void process_init() 
{
	int i;
	U32 *sp;
	PCB* temp;
  
        /* fill out the initialization table */
	set_test_procs();
	for ( i = 0; i < NUM_TEST_PROCS; i++ ) {
		g_proc_table[i].m_pid = g_test_procs[i].m_pid;
		g_proc_table[i].m_stack_size = g_test_procs[i].m_stack_size;
		g_proc_table[i].mpf_start_pc = g_test_procs[i].mpf_start_pc;
		// added a priority to the table
		g_proc_table[i].m_priority = g_test_procs[i].m_priority;
		printf("gtest_pcbs %d \n", (g_test_procs[i]).m_priority);
	}
  
	/* initilize exception stack frame (i.e. initial context) for each process */
	for ( i = 0; i < NUM_TEST_PROCS; i++ ) {
		int j;
		
		(gp_pcbs[i])->m_pid = (g_proc_table[i]).m_pid;
		// setting all processes to ready state in the beginning
		// adding all to the ready queue
		(gp_pcbs[i])->m_state = NEW;
		(gp_pcbs[i])->m_priority = (g_proc_table[i]).m_priority;
		(gp_pcbs[i])->next = NULL;
		
		printf("gp_pcbs %d \n", (gp_pcbs[i])->m_priority);
		rpq_enqueue(gp_pcbs[i]);
		
		sp = alloc_stack((g_proc_table[i]).m_stack_size);
		*(--sp)  = INITIAL_xPSR;      // user process initial xPSR  
		*(--sp)  = (U32)((g_proc_table[i]).mpf_start_pc); // PC contains the entry point of the process
		for ( j = 0; j < 6; j++ ) { // R0-R3, R12 are cleared with 0
			*(--sp) = 0x0;
		}
		(gp_pcbs[i])->mp_sp = sp;
	}
	
	temp = headReady;
	
	while(temp != NULL) {
			printf("temp %d \n", (temp)->m_pid);
			temp = temp->next;
	}
}

/*@brief: scheduler, pick the pid of the next to run process
 *@return: PCB pointer of the next to run process
 *         NULL if error happens
 *POST: if gp_current_process was NULL, then it gets set to pcbs[0].
 *      No other effect on other global variables.
 */

PCB *scheduler(void)
{
	return rpq_dequeue();
}

/*@brief: switch out old pcb (p_pcb_old), run the new pcb (gp_current_process)
 *@param: p_pcb_old, the old pcb that was in RUN
 *@return: RTX_OK upon success
 *         RTX_ERR upon failure
 *PRE:  p_pcb_old and gp_current_process are pointing to valid PCBs.
 *POST: if gp_current_process was NULL, then it gets set to pcbs[0].
 *      No other effect on other global variables.
 */
int process_switch(PCB *p_pcb_old) 
{
	PROC_STATE_E state;
	
	state = gp_current_process->m_state;

	if (state == NEW) {
		if (gp_current_process != p_pcb_old && p_pcb_old->m_state != NEW) {
			p_pcb_old->m_state = RDY;
			p_pcb_old->mp_sp = (U32 *) __get_MSP();
		}
		gp_current_process->m_state = RUN;
		__set_MSP((U32) gp_current_process->mp_sp);
		__rte();  // pop exception stack frame from the stack for a new processes
	} 
	
	/* The following will only execute if the if block above is FALSE */

	if (gp_current_process != p_pcb_old) {
		if (state == RDY){ 		
			p_pcb_old->m_state = RDY; 
			p_pcb_old->mp_sp = (U32 *) __get_MSP(); // save the old process's sp
			gp_current_process->m_state = RUN;
			__set_MSP((U32) gp_current_process->mp_sp); //switch to the new proc's stack    
		} else {
			gp_current_process = p_pcb_old; // revert back to the old proc on error
			return RTX_ERR;
		} 
	}
	return RTX_OK;
}

/**
 * @brief release_processor(). 
 * @return RTX_ERR on error and zero on success
 * POST: gp_current_process gets updated to next to run process
 */
int k_release_processor(void)
{
	PCB *p_pcb_old = NULL;
	
	PCB* temp = headReady;
	
	while(temp != NULL) {
			printf("release temp %d \n", (temp)->m_pid);
			temp = temp->next;
	}
	
	p_pcb_old = gp_current_process;
	gp_current_process = scheduler();
	printf("gp_current_process 0x%x \n", gp_current_process->m_state);
	
	if ( gp_current_process == NULL  ) {
		null_process();
//		gp_current_process = p_pcb_old; // revert back to the old process
		return RTX_ERR;
	}
  
	if ( p_pcb_old == NULL ) {
		p_pcb_old = gp_current_process;
	}
	
	process_switch(p_pcb_old);
	rpq_enqueue(p_pcb_old);
	return RTX_OK;
}
