#include "functions.h"
#include "task.h"
#include <avr/io.h>
#include "pwm.h"
static uint8_t counter = 0;

task_id_t counter_task_id = TASK_ID_INVALID;
task_id_t blink_task_id = TASK_ID_INVALID;
task_id_t breathe_task_id = TASK_ID_INVALID;
uint8_t brightness = 0;
int8_t brightness_step = BRIGHTNESS_STEP;

void counter_task(void)
{
    if (counter == 15)
    {
        counter = 0;
    }
    else
    {
        counter++;
    }

    PORTB = (PORTB & (uint8_t)~COUNTER_MASK) |
            (uint8_t)((counter & 0x0F) << COUNTER_BIT0);
}

void counter_task_cleanup(void){
    PORTB &= (uint8_t)~COUNTER_MASK;
    counter = 0;
}

void blink_task(void)
{
    PORTB ^= (1U << BLINK_LED);
}

void blink_task_cleanup(void){
    PORTB &= (uint8_t)~(1U << BLINK_LED);
}

void led_breathe_task(void)
{
    int16_t next = (int16_t)brightness + (int16_t)brightness_step;

    if (next >= 255)
    {
        brightness = 255;
        brightness_step = -BRIGHTNESS_STEP;
    }
    else if (next <= 0)
    {
        brightness = 0;
        brightness_step = BRIGHTNESS_STEP;
    }
    else
    {
        brightness = (uint8_t)next;
    }

    pwm_led_set_duty(brightness);
}

void led_breathe_task_cleanup(void){
    pwm_led_set_duty(0);
    brightness = 0;
    brightness_step = BRIGHTNESS_STEP;
}


const task_config_t counter_task_config = {
    .function = counter_task,
    .cleanup = counter_task_cleanup,
    .interval_ms = COUNTER_INTERVAL_MS,
    .policy = SCHEDULE_FIXED_RATE,
    .led_count = 4,
    .leds = {COUNTER_BIT0, COUNTER_BIT1, COUNTER_BIT2, COUNTER_BIT3},
    .port = B
    };

const task_config_t blink_task_config = {
    .function = blink_task,
    .cleanup = blink_task_cleanup,
    .interval_ms = BLINK_INTERVAL_MS,
    .policy = SCHEDULE_FROM_LAST_RUN,
    .led_count = 1,
    .leds = {BLINK_LED},
    .port = B
    };

const task_config_t breathe_task_config = {
    .function = led_breathe_task,
    .cleanup = led_breathe_task_cleanup,
    .interval_ms = BRIGHTNESS_UPDATE_MS,
    .policy = SCHEDULE_FIXED_RATE,
    .led_count = 1,
    .leds = {PD6},
    .port = D
    };