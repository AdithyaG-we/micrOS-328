#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

extern volatile uint32_t system_ticks;
extern volatile uint16_t timer1_overflow_count;

void timer_init(void);
uint32_t timer_get_ticks(void);
uint32_t timer_get_hardware_ticks(void);

#endif