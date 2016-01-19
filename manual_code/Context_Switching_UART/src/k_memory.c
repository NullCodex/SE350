/**
 * @file:   k_memory.c
 * @brief:  kernel memory managment routines
 * @author: Yiqing Huang
 * @date:   2014/01/17
 */

#include "k_memory.h"

#ifdef DEBUG_0
#include "printf.h"
#endif /* ! DEBUG_0 */

mem_block* head;
/* ----- Global Variables ----- */
U32 *gp_stack; /* The last allocated stack low address. 8 bytes aligned */
               /* The first stack starts at the RAM high address */
	       /* stack grows down. Fully decremental stack */

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
          |---------------------------|
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

void memory_init(void)
{
	U8 *p_end = (U8 *)&Image$$RW_IRAM1$$ZI$$Limit;
	int i;

	/* 4 bytes padding */
	p_end += 4;

	/* allocate memory for pcb pointers   */
	gp_pcbs = (PCB **)p_end;
	p_end += NUM_TEST_PROCS * sizeof(PCB *);

	for ( i = 0; i < NUM_TEST_PROCS; i++ ) {
		gp_pcbs[i] = (PCB *)p_end;
		p_end += sizeof(PCB);
	}
#ifdef DEBUG_0
	printf("gp_pcbs[0] = 0x%x \n", gp_pcbs[0]);
	printf("gp_pcbs[1] = 0x%x \n", gp_pcbs[1]);
#endif

	/* prepare for alloc_stack() to allocate memory for stacks */

	gp_stack = (U32 *)RAM_END_ADDR;
	if ((U32)gp_stack & 0x04) { /* 8 bytes alignment */
		--gp_stack;
	}

	/* allocate memory for heap, not implemented yet*/

    // Need to calculate the low address and the high address
    // Assume 30 blocks

    U32* curr_adress = gp_pcbs;
    curr_address += sizeof(mem_block *);
    mem_block* curr_node = (mem_block*)curr_address;
    curr_node->next = NULL;
    curr_node->block_address = gp_pcbs;
    head = curr_node;
    int j;
    for(j = 0; j < 29; j++) {
        curr_address += ( sizeof(mem_block*) + 128);
        curr_node = (mem_block*) curr_address;
        curr_node->next = head;
        curr_node->block_address = curr_node -= sizeof(mem_block*);
        head = curr_node;
    }

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

void *k_request_memory_block(void) {
#ifdef DEBUG_0
	printf("k_request_memory_block: entering...\n");
#endif /* ! DEBUG_0 */
	return (void *) NULL;

    void* mem_blk;
    // Rough idea of what we want to do
    __disable_irq(); //atomic(on);

    while(head == NULL) {
        k_release_processor();
    }

    mem_blk = (void*)head->block_address;
    head = head->next;
    __enable_irq(); //atomic (off)
    return mem_blk;
}

int k_release_memory_block(void *p_mem_blk) {
#ifdef DEBUG_0
	printf("k_release_memory_block: releasing block @ 0x%x\n", p_mem_blk);
#endif /* ! DEBUG_0 */
    __disable_irq(); //atomic(on);
    U32 release_address = (U32)p_mem_blk;
    if(release_address > low_adddress || release_address > high_address) {
        return RTX_ERR;
    }

    mem_block* node = (mem_block*)(release_address+sizeof(mem_block*);
    node->next = head;
    head = node;
    __enable_irq(); //atomic (off);
	return RTX_OK;
}
