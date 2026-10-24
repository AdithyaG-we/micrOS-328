#include <avr/io.h>
#include <stdint.h>
#include <stddef.h>
#include <avr/sleep.h>

#include "timer.h"
#include "scheduler.h"
#include "interrupt.h"
#include "task.h"
#include "pwm.h"
#include "functions.h"

scheduler_status_t status;
static uint8_t error_led_active = 0;
static uint32_t error_led_turn_off_time = 0;

static void gpio_init(void)
{
    DDRB |= COUNTER_MASK;
    DDRB |= (1U << ERROR_LED);
    DDRB |= (1U << BLINK_LED);
    DDRD &= ~(1 << PD2); // Set PD2 (INT0) as input
    DDRD &= ~(1 << PD3); // Set PD3 (INT1) as input

    PORTB &= (uint8_t)~COUNTER_MASK;
    PORTB &= (uint8_t)~(1U << BLINK_LED);
    PORTB &= (uint8_t)~(1U << ERROR_LED);
}

int main(void)
{
    gpio_init();
    timer_init();
    scheduler_init(&status);
    enable_INT0();
    enable_INT1();
    pwm_led_init();
    set_sleep_mode(SLEEP_MODE_IDLE);

    if (!scheduler_sanity_check())
    {
        // keeps the blink led ON to indicate a failure in the self-test.
        PORTB |= (1U << ERROR_LED);
        while (1)
        {
            // Handle error
        }
    }

    status = add_and_verify_task(&counter_task_config, &counter_task_id);
    if (status != SCHEDULER_OK)
    {
        // keeps the blink led ON to indicate a failure in the self-test.
        PORTB |= (1U << ERROR_LED);
        while (1)
        {
            // Handle error
        }
    }

    status = add_and_verify_task(&blink_task_config, &blink_task_id);
    if (status != SCHEDULER_OK)
    {
        // keeps the blink led ON to indicate a failure in the self-test.
        PORTB |= (1U << ERROR_LED);
        while (1)
        {
            // Handle error
        }
    }

    if (add_task(&breathe_task_config, &breathe_task_id) != SCHEDULER_OK)
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
    status = add_and_verify_task(&counter_task_config, &counter_task_id); // ensure correct task is added after deletion.
    volatile uint32_t dummy = 0;                                          // to get uneven spacing in the increment of counter.
    while (dummy < 30000)
    {
        // scheduler_run_once();
        dummy++;
    }
    status = add_and_verify_task(&counter_task_config, &counter_task_id); // ensure correct task is added after deletion.

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
        if (disable_breathing_requested == 1)
        {
            delete_one_task(breathe_task_id);
            disable_breathing_requested = 0;
        }
        if (disable_breathing_requested == 2)
        {
            status = add_and_verify_task(&breathe_task_config, &breathe_task_id);
            if (status != SCHEDULER_OK)
            {
                PORTB |= (1U << ERROR_LED);
                error_led_turn_off_time = timer_get_hardware_ticks() + ERROR_LED_TIME;
                error_led_active = 1;
            }
            disable_breathing_requested = 0;
        }
        if (error_led_active && (int32_t)(timer_get_hardware_ticks() - error_led_turn_off_time) >= 0)
        {
            PORTB &= (uint8_t)~(1U << ERROR_LED);
            error_led_active = 0;
        }
        scheduler_run_once();
        sleep_mode();
    }
}
