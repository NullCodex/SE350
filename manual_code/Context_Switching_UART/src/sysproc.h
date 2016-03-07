#ifndef SYS_PROC_H_
#define SYS_PROC_H_

#include "k_rtx.h"
#include "rtx.h"
#include "msg.h"
#include "common.h"
#include "string.h"

#define K_MSG_ENV

void kcd_proc(void);
void print_wall_clock(int, int, int);
void wall_clock(void);
#endif
