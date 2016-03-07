/**
 * @file:   usr_proc.c
 * @brief:  Six user processes: proc1...6 to test memory blocking/unblocking 
 * @author: Yiqing Huang
 * @date:   2014/02/07
 * NOTE: Each process is in an infinite loop. Processes never terminate.
 *       The test requires set_process_priority preemption works properly.
 *   
 * Two possible output unde the assumption that 
 * we have TWO memory blocks in the system.
 *
 * Expected UART output: (assuming memory block has ownership.):
 * ABCDE
 * FGHIJ
 * 01234
 * KLMNO
 * 56789
 * proc2: end of testing
 * proc3: 
 * proc4: 
 * proc5: 
 * proc6: 
 * proc3: 
 * proc4: 
 * proc5: 
 * proc6: 
 *
 * Expected UART output: (assuming shared memory among processes (no ownership))
 * ABCDE
 * FGHIJ
 * 01234
 * KLMNO
 * 56789
 * PQRST
 * 01234
 * UVWXY
 * 56789
 * ZABCD
 * 01234
 * ...... you see P1 and P2 keep alternating between each other, p3-p6 will never run
 * 
 */

#include "rtx.h"
#include "uart_polling.h"
#include "usr_proc.h"

#ifdef DEBUG_0
#include "printf.h"
#endif /* DEBUG_0 */

/* initialization table item */
PROC_INIT g_test_procs[NUM_TEST_PROCS];

void set_test_procs() {
	int i;
	for( i = 0; i < NUM_TEST_PROCS; i++ ) {
		g_test_procs[i].m_pid=(U32)(i+1);
		g_test_procs[i].m_stack_size=0x100;
	}
  
	g_test_procs[0].mpf_start_pc = &proc1;
	g_test_procs[0].m_priority   = LOW;
	
	g_test_procs[1].mpf_start_pc = &proc2;
	g_test_procs[1].m_priority   = LOW;
	
	g_test_procs[2].mpf_start_pc = &proc3;
	g_test_procs[2].m_priority   = HIGH;
	
	g_test_procs[3].mpf_start_pc = &proc4;
	g_test_procs[3].m_priority   = LOW;
	
	g_test_procs[4].mpf_start_pc = &proc5;
	g_test_procs[4].m_priority   = LOW;
	
	g_test_procs[5].mpf_start_pc = &proc6;
	g_test_procs[5].m_priority   = LOW;
}


/**
 * @brief: a process that prints five uppercase letters
 *         and request a memory block.
 */
void proc1(void)
{
	int i = 0;
	msgbuf* p_msg_env = request_memory_block();
	p_msg_env->mtype = DEFAULT;
	p_msg_env->mtext[0] = 'W';
	send_message(PID_CLOCK, (void *)p_msg_env);
	while ( 1 ) {
		if ( i != 0 && i%5 == 0 ) {
			uart0_put_string("\n\r");
		
		}
		uart0_put_char('A' + i%26);
		i++;
	}
}

/**
 * @brief: a process that prints five numbers
 *         and then releases a memory block
 */
void proc2(void)
{
	int i = 0;
	msgbuf* p_msg_env = request_memory_block();
	msgbuf* p_msg_term;
	p_msg_env->mtype = DEFAULT;
	p_msg_env->mtext[0] = 'W';
	send_message(PID_CLOCK, (void *)p_msg_env);
	while ( 1 ) {
		if ( i != 0 && i%5 == 0 ) {
			uart0_put_string("\n\r");
		}
		if( i != 0 && i % 1000 == 0) {
			p_msg_term = request_memory_block();
			p_msg_term->mtext[0] = 'W';
			p_msg_term->mtext[1] = 'T';
			send_message(PID_CLOCK, (void *)p_msg_term);
		}
		uart0_put_char('A' + i%26);
		i++;
	}
}


void proc3(void)
{
	int i = 0;
	msgbuf* p_msg_env = request_memory_block();
	msgbuf* p_msg_term;
	p_msg_env->mtype = DEFAULT;
	p_msg_env->mtext[0] = 'W';
	send_message(PID_CLOCK, (void *)p_msg_env);
	while ( 1 ) {
		if ( i != 0 && i%5 == 0 ) {
			uart0_put_string("\n\r");
		}
		if( i != 0 && i % 1000 == 0) {
			p_msg_term = request_memory_block();
			p_msg_term->mtext[0] = 'W';
			p_msg_term->mtext[1] = 'S';
			p_msg_term->mtext[2] = ' ';
			p_msg_term->mtext[3] = '1';
			p_msg_term->mtext[4] = '2';
			p_msg_term->mtext[5] = ':';
			p_msg_term->mtext[6] = '3';
			p_msg_term->mtext[7] = '5';
			p_msg_term->mtext[8] = ':';
			p_msg_term->mtext[9] = '0';
			p_msg_term->mtext[10] = '9';
			send_message(PID_CLOCK, (void *)p_msg_term);
		}
		uart0_put_char('A' + i%26);
		i++;
	}
}

void proc4(void)
{
	int i = 0;
	msgbuf* p_msg_env = request_memory_block();
	msgbuf* p_msg_term;
	p_msg_env->mtype = DEFAULT;
	p_msg_env->mtext[0] = 'W';
	send_message(PID_CLOCK, (void *)p_msg_env);
	while ( 1 ) {
		if ( i != 0 && i%5 == 0 ) {
			uart0_put_string("\n\r");
		}
		if( i != 0 && i % 1000 == 0) {
			p_msg_term = request_memory_block();
			p_msg_term->mtext[0] = 'W';
			p_msg_term->mtext[1] = 'R';
			send_message(PID_CLOCK, (void *)p_msg_term);
		}
		uart0_put_char('A' + i%26);
		i++;
	}
}
void proc5(void)
{
	int i=0;
	
	while(1) {
		if ( i < 2 )  {
			uart0_put_string("proc5: \n\r");
		}
		release_processor();
		i++;
	}
}
void proc6(void)
{
	int i=0;
	
	while(1) {
		if ( i < 2 )  {
			uart0_put_string("proc6: \n\r");
		}
		release_processor();
		i++;
	}
}
