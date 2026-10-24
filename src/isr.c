#include <avr/interrupt.h>
volatile uint8_t disable_counter_requested = 0; // compiler optimisation prevention.
volatile uint8_t disable_breathing_requested = 0;
volatile uint8_t disable_blinking_requested = 0;
volatile uint32_t system_ticks;

ISR(INT0_vect){
    disable_breathing_requested = 1;
}

ISR(INT1_vect){
    disable_breathing_requested = 2;
}

ISR(TIMER1_COMPA_vect)
{
    system_ticks++;
}
