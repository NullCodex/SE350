/**
 * @file:   k_memory.c
 * @brief:  kernel memory managment routines
 * @author: Yiqing Huang
 * @date:   2014/01/17
 */

#include "k_memory.h"
#include "msg.h"

#ifdef DEBUG_0
#include "printf.h"
#endif /* ! DEBUG_0 */

/* ----- Global Variables ----- */
U32 *gp_stack; /* The last allocated stack low address. 8 bytes aligned */
               /* The first stack starts at the RAM high address */
	       /* stack grows down. Fully decremental stack */

typedef struct mem_block {
	U32 addr;
	struct mem_block* next;
	int released;
} mem_block;

mem_block* headBlock = NULL;
extern PCB* headBlocked;
extern PCB* tailBlocked;
extern PCB* gp_current_process;
extern void rpq_enqueue(PCB*);

/**
 * @brief: Initialize RAM as follows:

0x10008000+---------------------------+ High Address
          |    Proc 1 STACK           |
          |---------------------------|
          |    Proc 2 STACK           |
          |---------------------------|<--- gp_stack
          |                           |
          |        HEAP               |
          |                           |
          |---------------------------|<-- p_end
          |        PCB 2              |
          |---------------------------|
          |        PCB 1              |
          |---------------------------|
          |        PCB pointers       |
          |---------------------------|<--- gp_pcbs
          |        Padding            |
          |---------------------------|
          |Image$$RW_IRAM1$$ZI$$Limit |
          |...........................|
          |       RTX  Image          |
          |                           |
0x10000000+---------------------------+ Low Address

*/

void print_free_blocks() {
	mem_block * temp = headBlock;
	int c = 0;
	while (temp != NULL) {
		printf("Count: %d Free mem-block address: @ 0x%x\n", c, temp->addr);
		temp = temp->next;
		c++;
	}
}

void memory_init(void)
{
	U8 *p_end = (U8 *)&Image$$RW_IRAM1$$ZI$$Limit;
	U32	iterAddress;
	int i;

	/* 4 bytes padding */
	p_end += 4;

	/* allocate memory for pcb pointers   */
	gp_pcbs = (PCB **)p_end;
	p_end += (NUM_TOTAL_PROCS + 1) * sizeof(PCB *);

	for ( i = 0; i < NUM_TOTAL_PROCS; i++ ) {
		gp_pcbs[i] = (PCB *)p_end;
		p_end += sizeof(PCB);
	}
	/*
#ifdef DEBUG_0
	printf("gp_pcbs[0] = 0x%x \n", gp_pcbs[0]);
	printf("gp_pcbs[1] = 0x%x \n", gp_pcbs[1]);
#endif*/

	/* prepare for alloc_stack() to allocate memory for stacks */

	gp_stack = (U32 *)RAM_END_ADDR;
	if ((U32)gp_stack & 0x04) { /* 8 bytes alignment */
		--gp_stack;
	}

	/* allocate memory for heap, not implemented yet*/

  //headBlock = (void*) (p_end + sizeof(mem_block*));
	headBlock = (mem_block*) (p_end + sizeof(mem_block*) + 3*sizeof(int) + sizeof(msgbuf*) + sizeof(Envelope*));
	headBlock->addr = ((U32) headBlock + sizeof(mem_block*) + 3*sizeof(int) + sizeof(msgbuf*) + sizeof(Envelope*));
	headBlock->released = 1;
	headBlock->next = NULL;
	iterAddress = (U32)headBlock;
	for (i = 0; i < 30; i++) {
		/*
		headBlock->next = (void *) (headBlock->addr + BLOCK_SIZE);
		headBlock->next->addr = ((U32) headBlock->next + sizeof(mem_block*) + 3*sizeof(int) + sizeof(msgbuf*) + sizeof(Envelope*));
		headBlock->next->released = 1;
		headBlock = headBlock->next;
		*/
		iterAddress = iterAddress + BLOCK_SIZE + sizeof(mem_block*);
		headBlock->next = (mem_block*)iterAddress;
		headBlock->next->addr = iterAddress + sizeof(mem_block*) + 3*sizeof(int) + sizeof(msgbuf*) + sizeof(Envelope*);
		headBlock->released = 1;
		headBlock = headBlock->next;
	}
	headBlock->next = NULL;
	headBlock = (mem_block*) (p_end + sizeof(mem_block*) + 3*sizeof(int) + sizeof(msgbuf*) + sizeof(Envelope*));
#ifdef DEBUG_0
	//print_free_blocks();
#endif
}

/**
 * @brief: allocate stack for a process, align to 8 bytes boundary
 * @param: size, stack size in bytes
 * @return: The top of the stack (i.e. high address)
 * POST:  gp_stack is updated.
 */

U32 *alloc_stack(U32 size_b)
{
	U32 *sp;
	sp = gp_stack; /* gp_stack is always 8 bytes aligned */

	/* update gp_stack */
	gp_stack = (U32 *)((U8 *)sp - size_b);

	/* 8 bytes alignement adjustment to exception stack frame */
	if ((U32)gp_stack & 0x04) {
		--gp_stack;
	}
	return sp;
}

void bpq_enqueue (PCB *current_process) {
	PCB* temp = headBlocked;
	PCB* prev = NULL;

	if (headBlocked == NULL) {
		headBlocked = tailBlocked = current_process;
		headBlocked->next = NULL;
	} else {
		if (headBlocked == tailBlocked) {
			if (headBlocked->m_priority <= current_process->m_priority) {
				headBlocked->next = current_process;
				tailBlocked = current_process;
			} else {
				current_process->next = tailBlocked;
				headBlocked = current_process;
				tailBlocked = current_process->next;
			}
		} else {
			while (temp != tailBlocked->next) {
				if (temp->m_priority <= current_process->m_priority) {
					if (temp == tailBlocked) {
						current_process->next = temp->next;
						temp->next = current_process;
						tailBlocked = current_process;
						break;
					}
					prev = temp;
					temp = temp->next;
				} else {
					if (headBlocked == temp) {
						headBlocked = current_process;
					}
					current_process->next = temp;
					prev->next = current_process;
					break;
				}
			}
		}
	}
}

PCB* bpq_dequeue(void) {
		PCB* temp;
		if(headBlocked) {
			temp = headBlocked;
			headBlocked = headBlocked->next;
			temp->next = NULL;
			return temp;
		}
		return NULL;
}

void *k_request_memory_block(void) {

	mem_block *toRet = headBlock;

#ifdef DEBUG_0
	printf("k_request_memory_block: entering...\n");

#endif /* ! DEBUG_0 */
	while (headBlock == NULL) {
		gp_current_process->m_state = BOR;
		bpq_enqueue(gp_current_process);

		k_release_processor();
	}
	toRet = headBlock;
	headBlock = headBlock->next;
	toRet->released = 0;
	//toRet->next = NULL;
	//print_free_blocks();
	return (void*) (toRet->addr);
}

int k_release_memory_block(void *p_mem_blk) {
	int i = 0;
	mem_block* curr_node;
	PCB* current_process = NULL;
	msgbuf* toBeCleared = (msgbuf*)p_mem_blk;

#ifdef DEBUG_0
	printf("k_release_memory_block: releasing block @ 0x%x\n", p_mem_blk);
#endif /* ! DEBUG_0 */
	if (p_mem_blk == NULL) {
		return RTX_ERR;
	}
	if((U32)p_mem_blk >= (U32)gp_stack || (U32)p_mem_blk < 0x10000414) {

		return RTX_ERR;
	}

	/*if((U32)headBlock >= (U32)p_mem_blk - sizeof(mem_block*)) {
		return RTX_ERR;
	}*/
	if (headBlocked != NULL ) {
		current_process = bpq_dequeue();
		current_process->m_state = RDY;
		rpq_enqueue(current_process);
	}
	
	/*if(toBeCleared->mtext[0]) {
		while(toBeCleared->mtext[i] != '\0') {
			toBeCleared->mtext[i] = '\0';
			i++;
		}
	}*/

	
	curr_node = (mem_block *)((U32)p_mem_blk - sizeof(mem_block*) - 3*sizeof(int) - sizeof(msgbuf*) - sizeof(Envelope*));
	//curr_node = (mem_block *)((U32)p_mem_blk);
	
	if (curr_node->released == 1) {

		return RTX_ERR;
	}
	curr_node->next = headBlock;
	curr_node->released = 1;
	headBlock = curr_node;
	if (current_process != NULL && (current_process->m_priority <= gp_current_process->m_priority)) {

		k_release_processor();
	}

	return RTX_OK;
}