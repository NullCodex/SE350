#ifndef MSG_H
#define MSG_H

#include "k_memory.h"
#include "k_rtx.h"
#include "printf.h"
#include "k_process.h"
#include "k_timer.h"

/* ----- Definitions ----- */
#define DEFAULT 0
#define KCD_REG 1

typedef struct MSGBUF {
    int mtype;
    char mtext[(128 - 4*sizeof(int) - sizeof(struct msgbuf*) - sizeof(struct Envelope*))/sizeof(char)];
} msgbuf;

int send_message(int, void*);

void* receive_message(int*);

int delayed_send(int, void*, int);

void push_mailBox(PCB*, struct Envelope*);
struct Envelope* dequeue_mailBox(PCB*);
#endif