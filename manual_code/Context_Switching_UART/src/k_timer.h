/**
 * @brief timer.h - Timer header file
 * @author Y. Huang
 * @date 2013/02/12
 */
#ifndef K_TIMER_H_
#define K_TIMER_H_

#include "k_rtx.h"
#include "stdint.h"

extern uint32_t timer_init ( uint8_t n_timer );  /* initialize timer n_timer */

void timer_i_process(void);

#endif /* ! _TIMER_H_ */
