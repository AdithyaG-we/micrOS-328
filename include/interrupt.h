#ifndef INTERRUPT_H
#define INTERRUPT_H
#include <stdint.h>
#include <avr/interrupt.h>

void enable_INT0(void);

void enable_INT1(void);

void disable_INT0(void);

void disable_INT1(void);

#endif