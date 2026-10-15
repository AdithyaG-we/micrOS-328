#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include "timer.h"
#define LED_TOGGLE_INTERVAL_MS 200UL
#define LED_PIN PB5
static void led_init(void)
{
    //configure the led pin as output.
    DDRB |= (1U << LED_PIN);
}

static void led_toggle(void)
{
    //toggle the led.
    PORTB ^= (1U << LED_PIN);
}

int main(void)
{
    led_init();
    timer_init();
    uint32_t last_toggle = timer_get_ticks();
    while (1)
    {
        uint32_t now = timer_get_ticks();
        if(now - last_toggle >= LED_TOGGLE_INTERVAL_MS){
            led_toggle();
            last_toggle = now;
        }
    }
    return 0;
}
