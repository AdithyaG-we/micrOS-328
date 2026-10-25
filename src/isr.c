#include <avr/interrupt.h>
#include "time.h"
volatile uint8_t disable_counter_requested = 0; // compiler optimisation prevention.
volatile uint8_t disable_breathing_requested = 0;
volatile uint8_t disable_blinking_requested = 0;
volatile uint32_t system_ticks;
static uint32_t last_trigger = 0;

ISR(INT0_vect){
    disable_breathing_requested = 1;
}

ISR(INT1_vect){
    uint32_t now = timer_get_hardware_ticks();
    if((uint32_t)(now - last_trigger) < MS_TO_TICKS(250)){
        return;
    }
    last_trigger = now;
    disable_breathing_requested = 2;
}

volatile uint16_t timer1_overflow_count = 0;
ISR(TIMER1_OVF_vect)
{
    timer1_overflow_count++;
}