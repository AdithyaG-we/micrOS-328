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

    /* Stop Timer1 while configuring it. */
    TCCR1A = 0;
    TCCR1B = 0;

    /* Reset the timer counter. */
    TCNT1 = 0;

    /* Set the compare value for a 1 ms interval. */
    OCR1A = TIMER1_COMPARE;

    /* Clear any pending Compare Match A interrupt flag. */
    TIFR1 = (1 << OCF1A);

    /* Enable Timer1 Compare Match A interrupt. */
    TIMSK1 = (1 << OCIE1A);

    /*
     * Configure CTC mode and select a prescaler of 64.
     * Writing CS12:CS10 as 011 selects the divide-by-64 clock.
     */
    TCCR1B = (1 << WGM12) |
             (1 << CS11) |
             (1 << CS10);

    /* Enable global interrupts. */
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