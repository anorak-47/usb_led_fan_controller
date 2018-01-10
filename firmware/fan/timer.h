#ifndef __TIMER_H_INCLUDED__
#define __TIMER_H_INCLUDED__

#include "config.h"
#include <inttypes.h>

#define LEDSTRIPE_DELAY_COUNT (FREQ_PWM / 1000 * 9)

extern uint16_t timer1_ovf_counter;
extern volatile uint8_t timer1_ledstripe_delay_counter;

void initTimer(void);

#endif
