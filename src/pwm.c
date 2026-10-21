#include <avr/io.h>
#include <stdint.h>

#include "pwm.h"

void pwm_led_init(void)
{
    /* OC0A is on PD6, not an Arduino Uno's built-in D13 LED. */
    DDRD |= (uint8_t)(1U << DDD6);
    PORTD &= (uint8_t)~(1U << PORTD6);

    /* Timer0 Fast PWM, TOP = 0xFF (WGM02:0 = 0b011).
       Start with OC0A disconnected so 0 duty is truly OFF. */
    TCCR0A = (uint8_t)((1U << WGM01) | (1U << WGM00));
    TCCR0B = (uint8_t)((1U << CS01) | (1U << CS00));
    TCNT0 = 0;
    OCR0A = 0;
}

void pwm_led_set_duty(uint8_t duty)
{
    if (duty == 0U)
    {
        /* In Fast PWM, OCR0A = 0 can otherwise produce a narrow pulse.
           Disconnect the timer and explicitly drive PD6 low. */
        TCCR0A &= (uint8_t)~((1U << COM0A1) | (1U << COM0A0));
        PORTD &= (uint8_t)~(1U << PORTD6);
        OCR0A = 0;
    }
    else
    {
        /* COM0A1:0 = 10: non-inverting hardware PWM on OC0A. */
        OCR0A = duty;
        TCCR0A = (uint8_t)((TCCR0A | (1U << COM0A1)) &
                           (uint8_t)~(1U << COM0A0));
    }
}
