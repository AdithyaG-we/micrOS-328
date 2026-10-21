#ifndef PWM_H
#define PWM_H

#include <stdint.h>

/*
 * Hardware PWM LED driver for ATmega328P.
 *
 * Output: PD6 / OC0A (Arduino D6; DIP-28 physical pin 12).
 * Timer:  Timer0, Fast PWM (8-bit), non-inverting, prescaler = 64.
 * PWM frequency with a 16 MHz CPU: 16 MHz / (64 * 256) = 976.56 Hz.
 *
 * Timer1 is deliberately not used: micrOS-328 reserves it for
 * the 1 ms scheduler tick in src/timer.c.
 *
 * Timer0 and OC0A/PD6 must not be configured by another driver.
 */

void pwm_led_init(void);

/* Duty: 0 = always OFF, 255 = fully ON, 1..254 = intermediate brightness. */
void pwm_led_set_duty(uint8_t duty);

#endif /* PWM_H */
