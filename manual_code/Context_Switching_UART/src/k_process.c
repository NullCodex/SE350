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
PCB *headBlockedMail = NULL;
extern PCB *clock_process;
extern Envelope* headTimer;

/* process initialization table */

PROC_INIT g_proc_table[NUM_TOTAL_PROCS];
extern PROC_INIT g_test_procs[NUM_TEST_PROCS];
PROC_INIT g_api_procs[NUM_API_PROCS];
extern PCB* timer_process;
extern PCB* uart_process;
//extern void timer_i_process(void);

/**
*	Null Process
*
*/

void mail_benqueue(PCB* process) {
	if(headBlockedMail) {
		process->next = headBlockedMail;
		headBlockedMail = process;
	} else {
		process->next = NULL;
		headBlockedMail = process;
	}
}

// Assuming the list is not empty
PCB* remove_from_mail_blocked(int pid) {
	PCB* prev = NULL;
	PCB* current = headBlockedMail;
	
	// only a single element
	if(headBlockedMail->m_pid == pid && headBlockedMail->next == NULL) {
		headBlocked = NULL;
		return current;
	} 
	// if we want to remove the first element, but there are more elements in the queue
	else if(headBlockedMail->m_pid == pid) {
		headBlockedMail = headBlockedMail->next;
		return current;
	}
	while(current) {
		if(current->m_pid == pid) {
			prev->next = current->next;
			current->next = NULL;
			return current;
		}
		prev = current;
		current = current->next;
	}
	
	return NULL;
}

// For hot keys
void printTimerBlockedQueue() {
	PCB* iter = headBlockedMail;
	if(!iter) {
		printf("Blocked on receive queue is empty \n");
	}
	else {
		while(iter) {
			printf("Current process in received blocked queue is: %d \n", iter->m_pid);
			iter = iter->next;
		}
	}
}

void printReadyQueue() {
	PCB* iter = headReady;
	if(!iter) {
		printf("Ready queue is empty \n");
	}
	else {
		while(iter) {
			printf("Current process in ready queue: %d \n", iter->m_pid);
			iter = iter->next;
		}
	}
}

void printTimeOutQueue() {
	Envelope* iter = headTimer;
	msgbuf* iterMessage = NULL;
	int i = 0;
	if(!iter) {
		printf("Timeout queue is empty \n");
	}
	else {
		while(iter) {
			printf("Current envelope message: ");
			iterMessage = (msgbuf*)iter->message;
			for(i = 0; i < sizeof(iterMessage->mtext)/sizeof(char); i++) {
				printf("%c", iterMessage->mtext[i]); 
			}
			printf("\n");
			iter = iter->next;
		}
	}
}

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
			temp->next = NULL;
			return temp;
		}
		return NULL;
}

PCB* getProcessByID(int process_id) {
	int i;
	
	for ( i = 0; i < NUM_TOTAL_PROCS; i++ ) {
		if(gp_pcbs[i]->m_pid == process_id) {
			return gp_pcbs[i];
		}
	}
	
	// return NULL if not found, according to specs
	return NULL;
}

// Removes process by ID by iterating through the ready queue
PCB* removeProcessByID(int pid) {
	PCB* temp;
	PCB* prev;
	PCB* blocked;
	
	// if the head of the ready queue is to be removed
	if(headReady->m_pid == pid) {
		return rpq_dequeue();
	}
	temp = headReady;
	
	// iterate
	while(temp->next != NULL) {
		if(temp->next->m_pid == pid) {
			blocked = temp->next;
			temp->next = temp->next->next;
			if (blocked == tailReady) {
				tailReady = temp;
			} 
			blocked->next = NULL;
			return blocked;
		}
		prev = temp;
		temp = temp->next;
	}
	
	// if the last element is to be removed
	if(tailReady->m_pid == pid) {
		blocked = tailReady;
		prev->next = NULL;
		tailReady = prev;
		return blocked;
	}
	
	// should never come here
	return NULL;
}

int k_get_process_priority(int process_id) {
	int i;
	
	for ( i = 0; i < NUM_TOTAL_PROCS; i++ ) {
		if(gp_pcbs[i]->m_pid == process_id) {
			return gp_pcbs[i]->m_priority;
		}
	}
	
	// return -1 if not found, according to specs
	return -1;
}

void rpq_enqueue (PCB *current_process) {
	PCB* temp = headReady;
	PCB* prev = NULL;
	if(headReady != current_process && tailReady != current_process) {
		
		if (headReady == NULL) {
			headReady = tailReady = current_process;
		} else {
			if (headReady == tailReady) {
				if (headReady->m_priority <= current_process->m_priority) {
					headReady->next = current_process;
					tailReady = current_process;
				} else {
					current_process->next = tailReady;
					headReady = current_process;
					tailReady = current_process->next;
				}
			} else {
				while (temp != tailReady->next) {
					if (temp->m_priority <= current_process->m_priority) {
						if (temp == tailReady) {
							current_process->next = temp->next;
							temp->next = current_process;
							tailReady = current_process;
							break;
						}
						prev = temp;
						temp = temp->next;
					} else {
						if (headReady == temp) {
							headReady = current_process;
							current_process->next = temp;
						} else {
							current_process->next = temp;
							prev->next = current_process;
						}
						break;
					}
				}
			}
		}
	}
}

int k_set_process_priority(int process_id, int priority) {
	// gets the process by ID
	PCB* process = getProcessByID(process_id);
	int old_p;

	// if the new priority is the same priority as the current process 
		if(priority != process->m_priority) {
			// remove the process from the ready queue (by id)
			removeProcessByID(process_id);

			old_p = process->m_priority;
			
			//set the new priority
			process->m_priority = priority;
			
			// if preemption has to take place (i.e., current process will have greater priority, 
			// insert into correct position based on new priority
			// if current process is moved to a lower priority, process switch will take care of enqueueing into 
			// the right position
			
	  		if(old_p >= priority) {
				rpq_enqueue(process);
		 	}
	//		if(priority != headReady->m_priority) {
				k_release_processor();
	//		}
		}
		
	// not sure what to return here
	return process->m_pid;
}



/**
 * @biref: initialize all processes in the system
 * NOTE: We assume there are only two user processes in the system in this example.
 */

void set_api_procs() {
	int i;

	g_api_procs[0].m_pid= PID_TIMER_IPROC;
	g_api_procs[0].m_stack_size=0x200;
	g_api_procs[0].mpf_start_pc = &timer_i_process;
	g_api_procs[0].m_priority   = HIGHEST;
	g_api_procs[0].is_i_process = TRUE;
	
	g_api_procs[1].m_pid= PID_UART_IPROC;
	g_api_procs[1].m_stack_size=0x200;
	g_api_procs[1].mpf_start_pc = &UART_iprocess;
	g_api_procs[1].m_priority   = HIGHEST;
	g_api_procs[1].is_i_process = TRUE;
	
	g_api_procs[2].m_pid = PID_KCD;
	g_api_procs[2].m_stack_size=0x200;
	g_api_procs[2].mpf_start_pc = &kcd_proc;
	g_api_procs[2].m_priority = HIGH;
	g_api_procs[2].is_i_process = FALSE;
	
	g_api_procs[3].m_pid= PID_CRT;
	g_api_procs[3].m_stack_size=0x200;
	g_api_procs[3].mpf_start_pc = &crt_proc;
	g_api_procs[3].m_priority   = HIGH;
	g_api_procs[3].is_i_process = FALSE;
	
	g_api_procs[4].m_pid= PID_CLOCK;
	g_api_procs[4].m_stack_size=0x200;
	g_api_procs[4].mpf_start_pc = &wall_clock;
	g_api_procs[4].m_priority   = HIGH;
	g_api_procs[4].is_i_process = FALSE;
}

void process_init() 
{
	int i;
	int j = 0;
	U32 *sp;
	PCB* temp;
  
        /* fill out the initialization table */

	set_test_procs();
	set_api_procs();
	for ( i = 0; i < NUM_TEST_PROCS; i++ ) {
		g_proc_table[i].m_pid = g_test_procs[i].m_pid;
		g_proc_table[i].m_stack_size = g_test_procs[i].m_stack_size;
		g_proc_table[i].mpf_start_pc = g_test_procs[i].mpf_start_pc;
		// added a priority to the table
		g_proc_table[i].m_priority = g_test_procs[i].m_priority;
		g_proc_table[i].is_i_process = FALSE;
	}
	
	for (; i < NUM_TOTAL_PROCS; i++, j++ ) {
		g_proc_table[i].m_pid = g_api_procs[j].m_pid;
		g_proc_table[i].m_stack_size = g_api_procs[j].m_stack_size;
		g_proc_table[i].mpf_start_pc = g_api_procs[j].mpf_start_pc;
		// added a priority to the table
		g_proc_table[i].m_priority = g_api_procs[j].m_priority;
		g_proc_table[i].is_i_process = g_api_procs[j].is_i_process;
	}
  
	/* initilize exception stack frame (i.e. initial context) for each process */

	for ( i = 0; i < NUM_TOTAL_PROCS; i++ ) {
		int j;
		
		(gp_pcbs[i])->m_pid = (g_proc_table[i]).m_pid;
		// setting all processes to ready state in the beginning
		// adding all to the ready queue
		(gp_pcbs[i])->m_state = NEW;
		(gp_pcbs[i])->m_priority = (g_proc_table[i]).m_priority;
		(gp_pcbs[i])->next = NULL;
		(gp_pcbs[i])->mailBox = NULL;
		
	
		if(g_proc_table[i].is_i_process == FALSE) {
				gp_pcbs[i]->m_state = NEW;
				rpq_enqueue(gp_pcbs[i]);
			
		} else {
			(gp_pcbs[i])->m_state = WAITING_FOR_INTERRUPT;
			if(gp_pcbs[i]->m_pid == PID_TIMER_IPROC) {
				timer_process = gp_pcbs[i];
			}
			if(gp_pcbs[i]->m_pid == PID_UART_IPROC) {
				uart_process = gp_pcbs[i];
			}
			if(gp_pcbs[i]->m_pid == PID_CLOCK) {
				clock_process = gp_pcbs[i];
			}
		}
		
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

		PCB* temp;
		
		if (gp_current_process != NULL && gp_current_process->m_state != BOR && gp_current_process->m_state != WFM) {
			temp = gp_current_process;
			temp->next = NULL;
			temp->m_state = RDY;
			rpq_enqueue(temp);
		}
		temp = rpq_dequeue();
		temp->next = NULL;
		//rpq_enqueue(temp);
		return temp;
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
		if (gp_current_process != p_pcb_old) {
			// last condition is for checking if p_pcb_old has the same priority as the head -> to avoid enqueueing twice
			// eg in premptionuserproc.c
			if (p_pcb_old->m_state != BOR && p_pcb_old->m_state != WFM && p_pcb_old->m_priority != headReady->m_priority ) {
				p_pcb_old->m_state = RDY;
				rpq_enqueue(p_pcb_old);
			}
			p_pcb_old->mp_sp = (U32 *) __get_MSP();
		}
		gp_current_process->m_state = RUN;
		
		__set_MSP((U32) gp_current_process->mp_sp);
		__rte();  // pop exception stack frame from the stack for a new processes
	} 
	
	/* The following will only execute if the if block above is FALSE */

	if (state == RDY) {
		if (gp_current_process != p_pcb_old) {

			if (p_pcb_old->m_state != BOR && p_pcb_old->m_state != WFM && p_pcb_old->m_priority != headReady->m_priority) {
				p_pcb_old->m_state = RDY; 
				rpq_enqueue(p_pcb_old);
			}
			p_pcb_old->mp_sp = (U32 *) __get_MSP(); // save the old process's sp
				gp_current_process->m_state = RUN;
			__set_MSP((U32) gp_current_process->mp_sp); //switch to the new proc's stack    
		}
	} else {
	//		printf("Testing error condition");
			gp_current_process = p_pcb_old; // revert back to the old proc on error
			return RTX_ERR;
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
	
	p_pcb_old = gp_current_process;
	
	gp_current_process = scheduler();

	if ( gp_current_process == NULL  ) {
		null_process();
//		gp_current_process = p_pcb_old; // revert back to the old process
		return RTX_ERR;
	}
  
	if ( p_pcb_old == NULL /*|| p_pcb_old->m_state==BOR*/ ) {
		p_pcb_old = gp_current_process;
	//	p_pcb_old->m_state = NEW;
	//	gp_current_process->m_state = NEW;
	} 
	
	// means that there is no other process with a higher priority on the ready queue
	// but we need to check if the old process is not a system process
	if(p_pcb_old->m_priority < gp_current_process->m_priority && p_pcb_old->m_pid < PID_SET_PRIO && p_pcb_old->m_state == RDY) {
		// since we're not switching to the low priority process, need to enqueue back
		rpq_enqueue(gp_current_process);
		gp_current_process = p_pcb_old;
	}

	process_switch(p_pcb_old);

	p_pcb_old = NULL;
	return RTX_OK;
}
