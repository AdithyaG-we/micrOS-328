#include <avr/io.h>
#include <stdint.h>
#include <stddef.h>
#include <avr/sleep.h>

#include "time.h"
#include "scheduler.h"
#include "interrupt.h"
#include "task.h"
#include "pwm.h"
#include "functions.h"

scheduler_status_t status;
static uint8_t error_led_active = 0;
static uint32_t error_led_turn_off_time = 0;

mutex_t portb_mutex;
mutex_t portc_mutex;
mutex_t portd_mutex;

uint32_t min(uint32_t a, uint32_t b){
    if(a <= b) return a;
    return b;
}

static void gpio_init(void)
{
    DDRB |= COUNTER_MASK;
    DDRB |= (1U << ERROR_LED);
    DDRB |= (1U << BLINK_LED);
    DDRD &= ~(1 << PD2); // Set PD2 (INT0) as input
    DDRD &= ~(1 << PD3); // Set PD3 (INT1) as input
    DDRD |= (1U << PWM_LED);
    DDRD |= (1U << POWER_ON_LED);

    PORTB &= (uint8_t)~COUNTER_MASK;
    PORTB &= (uint8_t)~(1U << BLINK_LED);
    PORTB &= (uint8_t)~(1U << ERROR_LED);
    PORTD &= (uint8_t)~(1U << PWM_LED);
    PORTD |= (uint8_t)(1U << POWER_ON_LED);
}

int main(void)
{
    gpio_init();
    timer_init();
    scheduler_init(&status);
    enable_INT0();
    enable_INT1();
    pwm_led_init();
    mutex_init(&portb_mutex);
    mutex_init(&portc_mutex);
    mutex_init(&portd_mutex);
    uint32_t cpu_wake_up_time;

    if (!scheduler_sanity_check())
    {
        PORTB |= (1U << ERROR_LED);
        while (1){}
    }

    status = add_and_verify_task(&counter_task_config, &counter_task_id);
    if (status != SCHEDULER_OK)
    {
        PORTB |= (1U << ERROR_LED);
        while (1){}
    }

    status = add_and_verify_task(&blink_task_config, &blink_task_id);
    if (status != SCHEDULER_OK)
    {
        PORTB |= (1U << ERROR_LED);
        while (1){}
    }

    if (add_task(&breathe_task_config, &breathe_task_id) != SCHEDULER_OK)
    {
        pwm_led_set_duty(0);
        PORTB |= (1U << ERROR_LED);
        for (;;){}
    }
    status = add_and_verify_task(&counter_task_config, &counter_task_id); // ensure correct task is added after deletion.

    if (status != SCHEDULER_OK)
    {
        PORTB |= (1U << ERROR_LED);
        while (1){}
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
        update_ready_list();
        scheduler_run_once();
        cpu_wake_up_time = determine_wake_up_time();
        sleep_until(min(cpu_wake_up_time, error_led_turn_off_time));
    }
    PORTD &= ~(1U << POWER_ON_LED);
}


// update the delete one task function to prevent deadlock caused due to mutex holding by removed task. done
// mutex waiting task id should be determined by task priority.
// equal priority tasks are being starved in scheduler_run_once()
