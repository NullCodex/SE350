/** 
 * @file:   queue.h
 * @brief:  kernel queue System
 * @auther: Akash Sant
 * @date:   2015/02/22
 */
 
#ifndef QUEUE_H_
#define QUEUE_H_

#include "k_rtx_init.h"
#include "k_rtx.h"
#include "k_memory.h"
/**
* Helper functoins for queue management
*/

typedef struct queue {
	PCB * head;
	PCB * tail;
} queue;

void enqueue(queue *, void *);
void * dequeue(queue *);
void * dequeueByID(queue* ,int);
void print_queue(queue **);

#endif // ! K_RTX_H_
