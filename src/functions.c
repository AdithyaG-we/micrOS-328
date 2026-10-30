#include "functions.h"
#include "task.h"
#include <avr/io.h>
#include "pwm.h"
static uint8_t counter = 0;

task_id_t counter_task_id = TASK_ID_INVALID;
task_id_t blink_task_id = TASK_ID_INVALID;
task_id_t breathe_task_id = TASK_ID_INVALID;
task_id_t led_pattern_task_id = TASK_ID_INVALID;
uint8_t brightness = 0;
int8_t brightness_step = BRIGHTNESS_STEP;
static uint8_t step = 0;

const uint8_t pattern[LED_PATTERN_STEPS] = {
    0x01, 0x0A, 0x04, 0x0F, /* 0001, 1010, 0100, 1111 */
    0x02, 0x09, 0x00, 0x06, /* 0010, 1001, 0000, 0110 */
    0x08, 0x03, 0x0C, 0x01, /* 1000, 0011, 1100, 0001 */
    0x07, 0x00, 0x0A, 0x05  /* 0111, 0000, 1010, 0101 */
};

void counter_task(void)
{
    if (counter == 15)
    {
        counter = 0;
    }
    else
    {
        // if(counter == 11){ // infinite loop added for visual demonstration.
        //     while(1){

        //     }
        // }
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

void led_pattern_task(void)
{
    task_entry_t* t = get_task_by_id(led_pattern_task_id);
    mutex_t* m = determine_mutex(led_pattern_task_id);
    PORTB = (PORTB & (uint8_t)~COUNTER_MASK) |
        (uint8_t)(pattern[step] << COUNTER_BIT0);
    ++step;
    if (step >= LED_PATTERN_STEPS) {
        PORTB &= (uint8_t)~(COUNTER_MASK);
        step = 0;

        if(m != NULL) mutex_unlock(m);
        if(t != NULL) t->state = TASK_BLOCKED;
    }
}

void led_pattern_cleanup(void)
{
    PORTB &= (uint8_t)~COUNTER_MASK;
    step = 0;
}

const task_config_t counter_task_config = {
    .function = counter_task,
    .cleanup = counter_task_cleanup,
    .interval_ms = COUNTER_INTERVAL_MS,
    .policy = SCHEDULE_FIXED_RATE,
    .priority = 2,
    .port = B,
    .multi_stage = 0
    };

const task_config_t blink_task_config = {
    .function = blink_task,
    .cleanup = blink_task_cleanup,
    .interval_ms = BLINK_INTERVAL_MS,
    .policy = SCHEDULE_FROM_LAST_RUN,
    .priority = 2,
    .port = B,
    .multi_stage = 0
    };

const task_config_t breathe_task_config = {
    .function = led_breathe_task,
    .cleanup = led_breathe_task_cleanup,
    .interval_ms = BRIGHTNESS_UPDATE_MS,
    .policy = SCHEDULE_FIXED_RATE,
    .priority = 1,
    .port = D,
    .multi_stage = 0
    };

const task_config_t led_pattern_task_config = {
    .function = led_pattern_task,
    .cleanup = led_pattern_cleanup,
    .interval_ms = MS_TO_TICKS(100),
    .policy = SCHEDULE_FIXED_RATE,
    .priority = 2,
    .port = B,
    .multi_stage = 1
};