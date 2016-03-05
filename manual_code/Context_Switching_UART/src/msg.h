
#ifndef MSG_H_
#define MSG_H_

#include "printf.h"

#include "timer.h"


#include "k_process.h"


/* ----- Definitions ----- */
#define DEFAULT 0
#define KCD_REG 1
#define CRT 		2




int k_send_message(int, void*);


void* k_receive_message(int*);


int k_delayed_send(int, void*, int);

void push_mailBox(PCB*, struct Envelope*);
struct Envelope* dequeue_mailBox(PCB*);
#endif