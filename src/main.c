#include <avr/io.h>
#include <stdint.h>
#include <stddef.h>

#include "timer.h"
#include "scheduler.h"
#include "interrupt.h"
#include "pwm.h"

#define COUNTER_BIT0 PB2
#define COUNTER_BIT1 PB3
#define COUNTER_BIT2 PB4
#define COUNTER_BIT3 PB5
#define ERROR_LED PB0

#define COUNTER_MASK ((1U << COUNTER_BIT0) | \
                      (1U << COUNTER_BIT1) | \
                      (1U << COUNTER_BIT2) | \
                      (1U << COUNTER_BIT3))

#define BLINK_LED PB1

#define COUNTER_INTERVAL_MS 1000UL
#define BLINK_INTERVAL_MS 300UL
/* Change these two values to adjust the breathing animation. */
#define BRIGHTNESS_STEP       3
#define BRIGHTNESS_UPDATE_MS 10UL

static uint8_t brightness = 0;
static int8_t brightness_step = BRIGHTNESS_STEP;

static uint8_t counter = 0;
scheduler_status_t status;
//static uint8_t disable_blink_requested = 0;
static uint8_t disable_counter_requested = 0;

static void counter_task(void)
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

static void blink_task(void)
{
    PORTB ^= (1U << BLINK_LED);
}

static void led_breathe_task(void)
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

const task_config_t counter_task_config = {
        .function = counter_task,
        .interval_ms = COUNTER_INTERVAL_MS,
        .policy = SCHEDULE_FIXED_RATE,
        .led_count = 4,
        .leds = {COUNTER_BIT0, COUNTER_BIT1, COUNTER_BIT2, COUNTER_BIT3},
    };

const task_config_t blink_task_config = {
    .function = blink_task,
    .interval_ms = BLINK_INTERVAL_MS,
    .policy = SCHEDULE_FROM_LAST_RUN,
    .led_count = 1,
    .leds = {BLINK_LED}};

const task_config_t breathe_task = {
        .function = led_breathe_task,
        .interval_ms = BRIGHTNESS_UPDATE_MS,
        .policy = SCHEDULE_FIXED_RATE,
        /* The current scheduler's LED cleanup only manipulates PORTB.
           PD6 is driven by Timer0, so don't register it in .leds. */
        .led_count = 0
    };


task_id_t counter_task_id;
task_id_t blink_task_id;
task_id_t breathe_task_id;

static void gpio_init(void)
{
    DDRB |= COUNTER_MASK;
    DDRB |= (1U << ERROR_LED);
    DDRB |= (1U << BLINK_LED);
    DDRD &= ~(1 << PD2); // Set PD2 (INT0) as input
    DDRD &= ~(1 << PD3); // Set PD3 (INT1) as input

    PORTB &= (uint8_t)~COUNTER_MASK;
    PORTB &= (uint8_t)~(1U << BLINK_LED);
    PORTB &= (uint8_t)(1U << ERROR_LED); // start as ON.
}

static uint8_t scheduler_sanity_check(void)
{
    uint32_t count;
    scheduler_status_t status;
    // ensure next task ID is set correctly after initialization.
    if (get_next_task_id() != 1)
    {
        return 0;
    }
    // ensure execution counts are zero at initialisation.
    for (uint8_t i = 1; i <= SCHEDULER_MAX_TASKS; i++)
    {
        status = scheduler_get_execution_count(i, &count);
        if (status == SCHEDULER_ERR_INVALID_ID) // passed.
        {
            continue;
        }
        else if (status != SCHEDULER_OK || count != 0) // passed.
        {
            return 0;
        }
    }

    // ensure invalid task IDs return the correct error code.
    status = scheduler_get_execution_count(SCHEDULER_MAX_TASKS + 1, &count);
    if (status != SCHEDULER_ERR_INVALID_ID)
    { // passed.
        return 0;
    }
    status = scheduler_get_execution_count(TASK_ID_INVALID, &count);
    if (status != SCHEDULER_ERR_INVALID_ID)
    { // passed.
        return 0;
    }

    // return 1 if all tests pass.
    return 1;
}

scheduler_status_t task_verification(const task_config_t *config, task_id_t *id)
{
    if (config->function == NULL || config->interval_ms == 0)
    {
        return SCHEDULER_ERR_INVALID_ARGUMENT;
    }
    scheduler_status_t status = scheduler_add_task(config, id);
    if (status != SCHEDULER_OK)
    {
        return status;
    }

    uint32_t count;
    status = scheduler_get_execution_count(*id, &count);
    if (status != SCHEDULER_OK || count != 0)
    {
        return SCHEDULER_ERR_INVALID_ARGUMENT;
    }

    return SCHEDULER_OK;
}

ISR(INT0_vect){
    disable_counter_requested = 1;
}

ISR(INT1_vect){
    disable_counter_requested = 2;
}

int main(void)
{
    gpio_init();
    timer_init();
    scheduler_init(&status);
    enable_INT0();
    enable_INT1();

    if (!scheduler_sanity_check())
    {
        // keeps the blink led ON to indicate a failure in the self-test.
        PORTB |= (1U << ERROR_LED);
        while (1)
        {
            // Handle error
        }
    }

    status = task_verification(&counter_task_config, &counter_task_id);
    if (status != SCHEDULER_OK)
    {
        // keeps the blink led ON to indicate a failure in the self-test.
        PORTB |= (1U << ERROR_LED);
        while (1)
        {
            // Handle error
        }
    }

    status = task_verification(&blink_task_config, &blink_task_id);
    if (status != SCHEDULER_OK)
    {
        // keeps the blink led ON to indicate a failure in the self-test.
        PORTB |= (1U << ERROR_LED);
        while (1)
        {
            // Handle error
        }
    }

    if (scheduler_add_task(&breathe_task, &breathe_task_id) != SCHEDULER_OK)
    {
        pwm_led_set_duty(0);
        PORTB |= (1U << ERROR_LED);
        for (;;)
        {
            /* Couldn't register breathing task. */
        }
    }
    uint32_t count = 0;
    while (count < 300000)
    {
        count++;
        scheduler_run_once();
    }
    delete_one_task(counter_task_id);
    count = 0;
    while (count < 300000)
    {
        scheduler_run_once();
        count++;
    }
    status = task_verification(&counter_task_config, &counter_task_id); // ensure correct task is added after deletion.
    volatile uint32_t dummy = 0;                                        // to get uneven spacing in the increment of counter.
    while (dummy < 30000)
    {
        // scheduler_run_once();
        dummy++;
    }
    status = task_verification(&counter_task_config, &counter_task_id); // ensure correct task is added after deletion.

    if (status != SCHEDULER_OK)
    {
        // keeps the blink led ON to indicate a failure in the self-test.
        PORTB |= (1U << ERROR_LED);
        while (1)
        {
            // Handle error
        }
    }
    while (1)
    {
        if(disable_counter_requested == 1){
            delete_one_task(counter_task_id);
            disable_counter_requested = 0;
        }
        if(disable_counter_requested == 2){
            task_verification(&counter_task_config, &counter_task_id);
            disable_counter_requested = 0;
        }
        scheduler_run_once();
    }
}
