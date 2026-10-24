#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>
extern volatile uint32_t system_ticks;

void timer_init(void);

uint32_t timer_get_ticks(void);

#endif