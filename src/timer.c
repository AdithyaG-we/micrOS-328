#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>

#include "timer.h"

#define TIMER1_PRESCALER 64UL
#define TIMER1_COMPARE   249UL  //250khz is the new frequency of the timer after prescaling, so 250khz/1000hz = 250

static volatile uint32_t system_ticks = 0;

ISR(TIMER1_COMPA_vect)
{
    system_ticks++;
}

void timer_init(void)
{
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        system_ticks = 0;
    }

    TCCR1A = 0;
    TCCR1B = 0;

    TCNT1 = 0;

    OCR1A = TIMER1_COMPARE;

    TIFR1 = (1 << OCF1A);

    TIMSK1 = (1 << OCIE1A);

    TCCR1B = (1 << WGM12) |
             (1 << CS11) |
             (1 << CS10);

    sei();
}

uint32_t timer_get_ticks(void)
{
    uint32_t ticks;

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        ticks = system_ticks;
    }

    return ticks;
}