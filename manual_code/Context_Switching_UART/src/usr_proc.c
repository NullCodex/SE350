/**
 G025_test: START 
G025_test: total 6
G025_test: Test 1 OK 
G025_test: Test 4 OK 
G025_test: Test 5 OK 
00:00:01
G025_test: Test 2 OK 
proc1: end of testing
01234
56789
12:35:09
proc5: 
proc5: 
proc5: 
proc5: 
proc5: 
.
.
.
.
*/

#include "rtx.h"
#include "uart_polling.h"
#include "usr_proc.h"

#ifdef DEBUG_0
#include "printf.h"
#endif /* DEBUG_0 */

/* initialization table item */
PROC_INIT g_test_procs[NUM_TEST_PROCS];
extern Element* pop(Queue*);
extern void push(Queue*, Element*);

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
	g_test_procs[2].m_priority   = LOW;
	
	g_test_procs[3].mpf_start_pc = &proc4;
	g_test_procs[3].m_priority   = LOW;
	
	g_test_procs[4].mpf_start_pc = &proc5;
	g_test_procs[4].m_priority   = LOW;
	
	g_test_procs[5].mpf_start_pc = &proc6;
	g_test_procs[5].m_priority   = LOW;

	g_test_procs[6].mpf_start_pc = &A;
	g_test_procs[6].m_priority   = HIGH;
	
	g_test_procs[7].mpf_start_pc = &B;
	g_test_procs[7].m_priority   = HIGH;
	
	g_test_procs[8].mpf_start_pc = &C;
	g_test_procs[8].m_priority   = HIGH;
}


/**
 * @brief: a process that prints five uppercase letters
 *         and request a memory block.
 */

void proc1(void)
{
	  int i=0;
		int retCode;
	  msgbuf *p_msg_env = request_memory_block();
    uart0_put_string("G025_test: START \n");
    uart0_put_string("G025_test: total ");
    uart0_put_char('0' + NUM_TEST_PROCS);
    uart0_put_string("\nG025_test: Test 1 OK \n");
   
    p_msg_env->mtype = DEFAULT;
    p_msg_env->mtext[0] = '%';
    p_msg_env->mtext[1] = 'W';
    p_msg_env->mtext[2] = '\0';
    retCode = delayed_send(PID_P2, (void *)p_msg_env, 1000);

    set_process_priority(PID_P2, HIGH);
    uart0_put_string("proc1: end of testing\n\r");
    while (1) {
        release_processor();
    }
   
}

void proc2(void)
{
	int sender_id;
	int i = 0;
	msgbuf *p_msg_env;
  uart0_put_string("G025_test: Test 2 OK \n");
	
	while(1) {
		p_msg_env = receive_message(&sender_id);
		uart0_put_string("G025_test: proc2: ");
		uart0_put_char(p_msg_env->mtext[0]);
		uart0_put_string("\n\r");
		if(i % 2 == 0) {
			uart0_put_string("proc2: \n\r");
			release_processor();
		}
	}
}

void proc3(void)
{
	int i = 0;
	void *p_mem_blk;
	uart1_put_string("G025_test: Test 3 OK \n");

	while ( 1 ) {
		if ( i < 1 ) {
	//		p_mem_blk = request_memory_block();
		}
		uart1_put_string("proc3: \n\r");
		release_processor();
		i++;
	}
}


void proc4(void)
{
	int i = 0;
	int ret_val = 20;
	void *p_mem_blk;
	uart0_put_string("G025_test: Test 4 OK \n");
	
	p_mem_blk = request_memory_block();
//	set_process_priority(PID_P3, MEDIUM);
	while ( 1) {
		if ( i < 1 ) {
			uart0_put_string("\n\r");
			ret_val = release_memory_block(p_mem_blk);

			if ( ret_val == -1 ) {
				break;
			}
		}
		release_processor();
		//uart0_put_char('0' + i%10);
		i++;
	}

	set_process_priority(PID_P4, LOWEST);
	while ( 1 ) {
		uart0_put_string("proc4: \n\r");
		release_processor();
	}
}

void proc5(void)
{
	int i = 0;

	msgbuf* p_msg_env = request_memory_block();
	msgbuf* p_msg_term;
	uart0_put_string("G025_test: Test 5 OK \n");
	
	p_msg_env->mtype = DEFAULT;
	p_msg_env->mtext[0] = '%';
	p_msg_env->mtext[1] = 'W';
	p_msg_env->mtext[2] = '\0';
	send_message(PID_CLOCK, (void *)p_msg_env);

	while ( 1 ) {
		if ( i < 1) {
			uart0_put_string("proc5: \n\r");
			p_msg_term = request_memory_block();
			p_msg_term->mtext[0] = '%';
			p_msg_term->mtext[1] = 'W';
			p_msg_term->mtext[2] = 'S';
			p_msg_term->mtext[3] = ' ';
			p_msg_term->mtext[4] = '1';
			p_msg_term->mtext[5] = '2';
			p_msg_term->mtext[6] = ':';
			p_msg_term->mtext[7] = '3';
			p_msg_term->mtext[8] = '5';
			p_msg_term->mtext[9] = ':';
			p_msg_term->mtext[10] = '0';
			p_msg_term->mtext[11] = '9';
			p_msg_term->mtext[12] = '\0';
			send_message(PID_CLOCK, (void *)p_msg_term);
		}
		release_processor();
		i++;
	}
}


void proc6(void)
{
	int i=0;
	msgbuf* p_msg_term = request_memory_block();
	
	uart0_put_string("G025_test: Test 6 OK \n");
	
	while(1) {
		if ( i < 1 ) {
			p_msg_term->mtext[0] = 'H';
			p_msg_term->mtext[1] = 'I';
			p_msg_term->mtext[2] = '\0';
			send_message(PID_CLOCK, (void *)p_msg_term);
		}
		uart1_put_string("proc6: \n\r");
		release_processor();
		i++;
	}
}

void A(void) //pid = 7
{
    msgbuf* msg;
    int num = 0;
    int sender_id;
    msg = request_memory_block();

    msg->mtype = KCD_REG;
    msg->mtext[0] = '%';
    msg->mtext[1] = 'Z';

    send_message(PID_KCD, msg);

    while(1){
	//			uart0_put_string("G025_test: A: ");
        msg = receive_message(&sender_id);
        if(msg->mtext[0] == '%' && msg->mtext[1] == 'Z'){
            break;
        }
        release_memory_block(msg);
    }

    release_memory_block(msg);

    while(1) {
//				uart0_put_string("G025_test: A: \n");
        msg = request_memory_block();
        msg->mtype = COUNT_REPORT;
        msg->mtext[0] = (char)num;
        send_message(8, msg);
        num = num + 1;
        release_processor();
    }

}

void B(void) //pid = 8
{
    msgbuf *msg;
    int sender_id;

    while(1){
	//			uart0_put_string("G025_test: B: \n");
        msg = receive_message(&sender_id);
        send_message(9, msg);
    }
}


void C(void) //pid == 9
{
    Queue q;
    int sender_id;
    msgbuf *msg;
    msgbuf *delay;
    msgbuf *receive;
    char print_msg[10] = {'P', 'r', 'o', 'c', 'e', 's', 's', ' ', 'C', '\0'};
    Element *element;
    int i = 0;
		int iter = 0;

    q.first = NULL;
    q.last = NULL;

    while(1) {
//				uart0_put_string("G025_test: C: \n");
        if(q.first == NULL){
            msg = receive_message(&sender_id);
        } else {
            element = pop(&q);
            msg = (msgbuf *)(element->data);
            element->data = NULL;
            release_memory_block(element);
        }
        if(msg->mtype == COUNT_REPORT && (int)(msg->mtext[0]) % 20 == 0){
            msg->mtype = CRT_DISPLAY;
						for(iter = 0; iter < 9; iter++) {
							msg->mtext[iter] = print_msg[iter];
						}
            send_message(PID_CRT, msg);

            delay = request_memory_block();
            delay->mtype = wakeup10;
            delay->mtext[0] = NULL;
            delayed_send(9, delay, 10000);
            while(1) {

                receive = receive_message(&sender_id);
                if(receive->mtype == wakeup10) {
                    release_memory_block(receive);
                    break;
                } else {
                        element = request_memory_block();
                        element->data = receive;
                        push(&q, element);
                }
            }


        } else {
            release_memory_block(msg);
        }
    }
}