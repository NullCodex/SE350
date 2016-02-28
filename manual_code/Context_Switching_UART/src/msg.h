#ifndef MSG_H_
#define MSG_H

#include "k_memory.h"

/* ----- Definitions ----- */
#define DEFAULT 0
#define KCD_REG 1

typedef struct msgbuf msgbuf;

struct msgbuf {
    int mtype;
    char mtext[(BLOCK_SIZE - 4*sizeof(int) - sizeof(msgbuf*) - sizeof(Envelope*))/sizeof(char)];
};

int send_message(int, void*);

void* receive_message(int*);

int delayed_send(int, void*, int);
#endif