#include "msg.h"
#include "k_rtx.h"
#include "k_memory.h"
#include "printf.h"
#include "k_process.h"
#include "timer.h"

extern PCB* gp_current_process;

int send_message(int process_id, void* message_envelope) {
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

    if(receiving_proc->state == WFM) {
        receiving_proc->state = RDY;
        rpq_enqueue(receiving_proc);
    }
    __enable_irq();

    return RTX_OK;
}

void* receive_message(int* sender_id) {
    Envelope* received;
    msgbuf* message;


    while(process->mailBox is empty) {
        gp_current_process->state = WFM;
        // probably need another queue for blocked on mail
        // push it here
        k_release_processor();
    }

    __disable_irq();

    received = pop mailque;
    message = received->message;

    *sender_id = received->send_id;

    __enable_irq();

    return (void*)message;

}

int delayed_send(int process_id, void* message_envelope, int delay) {
    PCB* receiving_proc = getProcessByID(process_id);
    Envelope* env;

    __disable_irq();
    env = (Envelope*) ((U32)message_envelope - 3*sizeof(int) - sizeof(msgbuf*) - sizeof(Envelope*));
    env->sender_id = gp_current_process->m_pid;
    env->destination_id = process_id;
    env->delay = delay;
    env->message = message_envelope;

    // after timer expires then we can push
    // create new mailbox for each pcb
    // enqueue into mailbox

    if(receiving_proc->state == WFM) {
        receiving_proc->state = RDY;
        rpq_enqueue(receiving_proc);
    }
    __enable_irq();

    return RTX_OK;
}

