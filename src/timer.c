#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>

#include "timer.h"

#define TIMER1_PRESCALER 64UL
//#define TIMER1_COMPARE   249UL  //250khz is the new frequency of the timer after prescaling, so 250khz/1000hz = 250

void timer_init(void)
{
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        system_ticks = 0;
    }

    TCCR1A = 0;
    TCCR1B = 0;

    TCNT1 = 0;

    //OCR1A = TIMER1_COMPARE;

    TIFR1 = (1 << OCF1A);

    TIMSK1 = (1 << TOIE1);

    TCCR1B = (1 << CS11) |
             (1 << CS10);

    sei();
}

uint32_t timer_get_ticks(void)
{
    uint32_t ticks;

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) //avoiding race condition.
    {
        ticks = system_ticks;
    }

    return ticks;
}

uint32_t timer_get_hardware_ticks(void){
    uint32_t high;
    uint16_t low;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
        low = TCNT1;
        high = timer1_overflow_count;
        if(TIFR1 & (1 << TOV1) && low < 32768){
            high++;
        }
    }
    return (uint32_t)((high << 16) | low) / 250UL;
}