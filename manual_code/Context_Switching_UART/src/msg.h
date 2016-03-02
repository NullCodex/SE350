#ifndef MSG_H
#define MSG_H

#include "k_memory.h"

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
#endif