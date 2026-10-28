#include "led_pattern.h"
#include "functions.h"
#include "time.h"
#include <avr/io.h>
#include <stdint.h>

/* Active-high LEDs: PB2=LED1, PB3=LED2, PB4=LED3, PB5=LED4.
 * Nibbles below read LED4 LED3 LED2 LED1. Deliberate jumps and
 * repeats create an irregular sequence instead of a binary count.
 */
static const uint8_t pattern[LED_PATTERN_STEPS] = {
    0x01, 0x0A, 0x04, 0x0F, /* 0001, 1010, 0100, 1111 */
    0x02, 0x09, 0x00, 0x06, /* 0010, 1001, 0000, 0110 */
    0x08, 0x03, 0x0C, 0x01, /* 1000, 0011, 1100, 0001 */
    0x07, 0x00, 0x0A, 0x05  /* 0111, 0000, 1010, 0101 */
};
static uint8_t step = 0;
task_id_t led_pattern_task_id = TASK_ID_INVALID;

void led_pattern_cleanup(void)
{
    /* Preserve error/blink LEDs and all other port bits. */
    PORTB &= (uint8_t)~COUNTER_MASK;
    step = 0;
}

void led_pattern_init(void)
{
    /* Call at startup, before registering this task. */
    led_pattern_cleanup();
    DDRB |= COUNTER_MASK;
}

void led_pattern_task(void)
{
    /* One non-blocking update; scheduler handles the delay. */
    PORTB = (PORTB & (uint8_t)~COUNTER_MASK) |
            (uint8_t)(pattern[step] << COUNTER_BIT0);
    ++step;
    if (step >= LED_PATTERN_STEPS) {
        step = 0;
    }
}

const task_config_t led_pattern_task_config = {
    .function = led_pattern_task,
    .cleanup = led_pattern_cleanup,
    /* This repo's interval_ms field actually holds timer ticks. */
    .interval_ms = MS_TO_TICKS(LED_PATTERN_STEP_MS),
    .policy = SCHEDULE_FIXED_RATE,
    .priority = 2,
    .port = B
};
