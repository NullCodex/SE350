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

typedef struct Envelope Envelope;
struct Envelope{
    int sender_id;
    int destination_id;
    int delay;
    msgbuf *message;
    struct Envelope* next;
};

int send_message(int, void*);

void* receive_message(int*);

int delayed_send(int, void*, int);
#endif