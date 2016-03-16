#ifndef MSG_H_
#define MSG_H_

#include "printf.h"
#include "k_process.h"
#include "timer.h"

/* ----- Definitions ----- */
#define DEFAULT 0
#define KCD_REG 1
#define CRT 		2

/*typedef struct MSGBUF {
    int mtype;
    char mtext[(128 - 4*sizeof(int) - sizeof(struct msgbuf*) - sizeof(struct Envelope*))/sizeof(char)];
} msgbuf;*/

int k_send_message(int, void*);

void* k_receive_message(int*);

void* k_non_blocking_receive_message(int*);

int k_delayed_send(int, void*, int);

void push_mailBox(PCB*, struct Envelope*);

int k_send_message_from_uart(int, int, void*);

Envelope* dequeue_mailBox(PCB*);
#endif
