#include <avr/io.h>
#include <util/delay.h>
#define F_CPU 16000000UL
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

    while (1)
    {
        led_toggle();
        _delay_ms(500);
    }

    return 0;
}
