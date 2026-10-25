#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>

#include "time.h"

#define TIMER1_PRESCALER 64U
//#define TIMER1_COMPARE 124U //triggers interrupt every 0.5ms.

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

    TCCR1B = (1 << CS11) | // not in ctc mode.
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
    return (uint32_t)((high << 16) | low);
}