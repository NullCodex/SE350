#ifndef SYS_PROC_H_
#define SYS_PROC_H_

#include "k_rtx.h"
#include "msg.h"
#include "common.h"

void crt_proc(void);
void UART_iprocess(void);
void kcd_proc(void);
void print_wall_clock(int, int, int);
void wall_clock(void);
#endif
