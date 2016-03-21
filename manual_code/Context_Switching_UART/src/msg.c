#include "msg.h"
#include "k_rtx.h"

extern PCB* gp_current_process;
extern void rpq_enqueue(PCB*);

int k_send_message(int process_id, void* message_envelope) {
    PCB* receiving_proc = getProcessByID(process_id);
    Envelope* env;
		msgbuf* msg = (msgbuf*) message_envelope;
	#ifdef DEBUG_0
		printf("Entering send message:\n\r");
	#endif
    env = (Envelope*) ((U32)message_envelope - 3*sizeof(int) - sizeof(msgbuf*) - sizeof(Envelope*));
		env->sender_id = gp_current_process->m_pid;
    env->destination_id = process_id;
    env->delay = 0;
    env->message = msg;

    // create new mailbox for each pcb
    // enqueue into mailbox
		push_mailBox(receiving_proc, env);

    if(receiving_proc->m_state == WFM) {
			#ifdef DEBUG_0
				printf("Receinng process %d is on WFM.\n\r", receiving_proc->m_pid);
			#endif
				receiving_proc = remove_from_mail_blocked(receiving_proc->m_pid);
				receiving_proc->m_state = RDY;
        rpq_enqueue(receiving_proc);
			
				if (receiving_proc->m_priority <= gp_current_process->m_priority) {
					#ifdef DEBUG_0
					printf("Prepare to prempt receiving process %d with new process %d.\n\r", receiving_proc->m_pid, gp_current_process->m_pid);
					#endif
					k_release_processor();
				}
    }

    return RTX_OK;
}

int k_send_message_from_uart(int sender_id, int process_id, void* message_envelope)  {
	  PCB* receiving_proc = getProcessByID(process_id);
    Envelope* env;
		msgbuf* msg = (msgbuf*) message_envelope;
	#ifdef DEBUG_0
		printf("Entering send message for UART:\n\r");
	#endif
    env = (Envelope*) ((U32)message_envelope - 3*sizeof(int) - sizeof(msgbuf*) - sizeof(Envelope*));
		env->sender_id = sender_id;
    env->destination_id = process_id;
    env->delay = 0;
    env->message = msg;

		push_mailBox(receiving_proc, env);

    if(receiving_proc->m_state == WFM) {
			#ifdef DEBUG_0
				printf("Receinng process %d is on WFM.\n\r", receiving_proc->m_pid);
			#endif
				receiving_proc = remove_from_mail_blocked(receiving_proc->m_pid);
				receiving_proc->m_state = RDY;
        rpq_enqueue(receiving_proc);
			
				if (receiving_proc->m_priority <= gp_current_process->m_priority) {
					#ifdef DEBUG_0
					printf("Prepare to prempt receiving process %d with new process %d.\n\r", receiving_proc->m_pid, gp_current_process->m_pid);
					#endif
					k_release_processor();
				}
    }

    return RTX_OK;
}

void* k_receive_message(int* sender_id) {
    Envelope* received;
    msgbuf* message;
	#ifdef DEBUG_0
		printf("Entering receive message:\n\r");
	#endif
    while(gp_current_process->mailBox == NULL) {
			#ifdef DEBUG_0
				printf("Put current process %d on WFM.\n\r", gp_current_process->m_pid);
			#endif
        gp_current_process->m_state = WFM;
        // probably need another queue for blocked on mail
        // push it here
				mail_benqueue(gp_current_process);

        k_release_processor();
    }

    //received = pop mailque;
		received = dequeue_mailBox(gp_current_process);
    message = (msgbuf* )received->message;
		
    *sender_id = received->sender_id;
		#ifdef DEBUG_0
		printf("Message recieved successfull.\n\r");
		#endif
    return (void*)message;
}


void* k_non_blocking_receive_message(int* sender_id) {
    Envelope* received;
    msgbuf* message;

    //received = pop mailque;
		if(gp_current_process->mailBox == NULL) {
			return NULL;
		}
		received = dequeue_mailBox(gp_current_process);
    message = (msgbuf* )received->message;

    *sender_id = received->sender_id;

    return (void*)message;
}

int k_delayed_send(int process_id, void* message_envelope, int delay) {
		PCB* receiving_proc = getProcessByID(process_id);
    Envelope* env;
		msgbuf* msg;
	#ifdef DEBUG_0
		printf("Entering delayed send, sending to process %d, sending from: %d \n\r", process_id, gp_current_process->m_pid);
	#endif
    env = (Envelope*) ((U32)message_envelope - 3*sizeof(int) - sizeof(msgbuf*) - sizeof(Envelope*));
    env->sender_id = gp_current_process->m_pid;
    env->destination_id = process_id;
    env->delay = delay;
    env->message = message_envelope;
		msg = (msgbuf*)env->message;
		timer_enqueue(env);

    return RTX_OK;
}


void push_mailBox(PCB* process, Envelope* env) {
	#ifdef DEBUG_0
	printf("Pushing into mailBox of process %d.\n\r", process->m_pid);
	#endif
	env->next = process->mailBox;
	process->mailBox = env;
}

// Asssuming that the mailbox is not empty
Envelope* dequeue_mailBox(PCB* process) {
	Envelope* message = process->mailBox;
	process->mailBox = process->mailBox->next;
	#ifdef DEBUG_0
	printf("Dequeue mailbox for process: %d\n\r", process->m_pid);
	#endif
	return message;
}
