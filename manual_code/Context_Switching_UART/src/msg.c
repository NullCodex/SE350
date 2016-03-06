#include "msg.h"
#include "k_rtx.h"

extern PCB* gp_current_process;

int k_send_message(int process_id, void* message_envelope) {
    PCB* receiving_proc = getProcessByID(process_id);
    Envelope* env;

    __disable_irq();
    env = (Envelope*) ((U32)message_envelope - 3*sizeof(int) - sizeof(msgbuf*) - sizeof(Envelope*));
    env->sender_id = gp_current_process->m_pid;
    env->destination_id = process_id;
    env->delay = 0;
    env->message = message_envelope;

    // create new mailbox for each pcb
    // enqueue into mailbox
		push_mailBox(receiving_proc, env);

    if(receiving_proc->m_state == WFM) {
			
			// we need to call k_release_processor() here <----- FOR JAMESON
        receiving_proc->m_state = RDY;
        rpq_enqueue(receiving_proc);
    }
    __enable_irq();

    return RTX_OK;
}

void* k_receive_message(int* sender_id) {
    Envelope* received;
    msgbuf* message;

    while(gp_current_process->mailBox == NULL) {
        gp_current_process->m_state = WFM;
        // probably need another queue for blocked on mail
        // push it here
				mail_benqueue(gp_current_process);
        k_release_processor();
    }

    __disable_irq();

    //received = pop mailque;
		received = dequeue_mailBox(gp_current_process);
    message = (msgbuf* )received->message;

    *sender_id = received->sender_id;

    __enable_irq();

    return (void*)message;
}


void* k_non_blocking_receive_message(int* sender_id) {
    Envelope* received;
    msgbuf* message;
    __disable_irq();

    //received = pop mailque;
		if(gp_current_process->mailBox == NULL) {
			return NULL;
		}
		received = dequeue_mailBox(gp_current_process);
    message = (msgbuf* )received->message;

    *sender_id = received->sender_id;

    __enable_irq();

    return (void*)message;
}

int k_delayed_send(int process_id, void* message_envelope, int delay) {
		PCB* receiving_proc = getProcessByID(process_id);
    Envelope* env;

    __disable_irq();
    env = (Envelope*) ((U32)message_envelope - 3*sizeof(int) - sizeof(msgbuf*) - sizeof(Envelope*));
    env->sender_id = gp_current_process->m_pid;
    env->destination_id = process_id;
    env->delay = delay;
    env->message = message_envelope;
		timer_enqueue(env);
		
    __enable_irq();

    return RTX_OK;
}


void push_mailBox(PCB* process, Envelope* env) {
	env->next = process->mailBox;
	process->mailBox = env;
}

// Asssuming that the mailbox is not empty
Envelope* dequeue_mailBox(PCB* process) {
	Envelope* message = process->mailBox;
	process->mailBox = process->mailBox->next;
	return message;
}
