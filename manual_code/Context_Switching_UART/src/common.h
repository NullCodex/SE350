/* @brief: common defines and structs for both kernel and user 
 * @file: common.h 
 * @author: Yiqing Huang
 * @date: 2016/02/24
 */

#ifndef COMMON_H_
#define COMMON_H_

/* Definitions */

#define BOOL unsigned char

#define TRUE 1
#define FALSE 0
#define NULL 0
#define RTX_ERR -1
#define RTX_OK 0
#define NUM_TEST_PROCS 6
#define NUM_API_PROCS 1
#define NUM_TOTAL_PROCS 7

/* Process IDs */
#define PID_NULL 0
#define PID_P1   1
#define PID_P2   2
#define PID_P3   3
#define PID_P4   4
#define PID_P5   5
#define PID_P6   6
#define PID_A    7
#define PID_B    8
#define PID_C    9
#define PID_SET_PRIO     10
#define PID_CLOCK        11
#define PID_KCD          12
#define PID_CRT          13
#define PID_TIMER_IPROC  14
#define PID_UART_IPROC   15


/* Process Priority. The bigger the number is, the lower the priority is*/
#define HIGHEST -1
#define HIGH    0
#define MEDIUM  1
#define LOW     2
#define LOWEST  3

/* Message Types */
#define DEFAULT 0
#define KCD_REG 1
#define CRT_DISPLAY 2

/* ----- Types ----- */
typedef unsigned char U8;
typedef unsigned int U32;

/* common data structures in both kernel and user spaces */

/* initialization table item */
typedef struct proc_init
{
	int m_pid;	        /* process id */
	int m_priority;         /* initial priority, not used in this example. */
	int m_stack_size;       /* size of stack in words */
	int is_i_process; 			/* flag for checking if i process */
	void (*mpf_start_pc) ();/* entry point of the process */
} PROC_INIT;

/* message buffer */
typedef struct MSGBUF
{
	int mtype;              /* user defined message type */
	char mtext[1];          /* body of the message */
} msgbuf;

typedef struct Envelope{
    int sender_id;
    int destination_id;
    int delay;
    struct msgbuf* message;
    struct Envelope* next;
} Envelope;



#endif // COMMON_H_
