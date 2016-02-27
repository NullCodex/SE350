/** 
 * @file:   queue.c
 * @brief:  Queue Managment System
 * @auther: Akash Sant
 * @date:   2015/02/22
 */

#include "queue.h"
#include "printf.h"

//TODO: Follows the PCB struct...change it to generalize and work with memory
void enqueue (queue * queue_t, void * toA) {
	PCB* toAdd = (PCB*) toA;
	if (queue_t->head == NULL) {
		queue_t->head = toAdd;
		queue_t->tail = queue_t->head;
	} else {
		queue_t->tail->next = toAdd;
		queue_t->tail = toAdd;
	}
}

/**
* Compare process priorities before enqueuing on 
*/
void * dequeue (queue * queue_t) {
	void * toRet = queue_t->head;
	queue_t->head = queue_t->head->next;
	return toRet;
}

void * dequeueByID (queue* queue_t, int pid) {
	PCB* temp;
	PCB* prev;
	PCB* blocked;
	
	// if the head of the ready queue is to be removed
	if(queue_t->head->m_pid == pid) {
		return dequeue(queue_t);
	}
	temp = queue_t->head;
	
	// iterate
	while(temp->next != NULL) {
		if(temp->next->m_pid == pid) {
			blocked = temp->next;
			temp->next = temp->next->next;
			blocked->next = NULL;
			return blocked;
		}
		prev = temp;
		temp = temp->next;
	}
	
	// if the last element is to be removed
	if(queue_t->tail->m_pid == pid) {
		blocked = queue_t->tail;
		prev->next = NULL;
		queue_t->tail = prev;
		return blocked;
	}
	
	// should never come here
	return NULL;
}

void print_queue(queue ** q) {
	int size = sizeof(q)/sizeof(q[0]);
	printf("Size of queue, %d \n", size);
	
}
