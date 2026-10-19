#include <avr/io.h>
#include <stdint.h>
#include <stddef.h>

#include "timer.h"
#include "scheduler.h"

#define COUNTER_BIT0 PB2
#define COUNTER_BIT1 PB3
#define COUNTER_BIT2 PB4
#define COUNTER_BIT3 PB5

#define COUNTER_MASK ((1U << COUNTER_BIT0) | \
(1U << COUNTER_BIT1) | \
(1U << COUNTER_BIT2) | \
(1U << COUNTER_BIT3))

#define BLINK_LED PB0

#define COUNTER_INTERVAL_MS 1000UL
#define BLINK_INTERVAL_MS   500UL

static uint8_t counter = 0;

static void gpio_init(void)
{
DDRB |= COUNTER_MASK;

DDRB |= (1U << BLINK_LED);

PORTB &= (uint8_t)~COUNTER_MASK;
PORTB &= (uint8_t)~(1U << BLINK_LED);

}

static void counter_task(void)
{
if(counter == 15)
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

static uint8_t scheduler_self_test(task_id_t counter_task_id, task_id_t blink_task_id)
{
    uint32_t count;
    scheduler_status_t status;
    if(counter_task_id != 1 || blink_task_id != 2) //passed.
    {
        return 0;
    }
    status = scheduler_get_execution_count(counter_task_id, &count);
    if(status != SCHEDULER_OK || count != 0){
        return 0;
    }
    status = scheduler_get_execution_count(blink_task_id, &count);
    if(status != SCHEDULER_OK || count != 0){
        return 0;
    }
    status = scheduler_get_execution_count(TASK_ID_INVALID, &count);
    if(status != SCHEDULER_ERR_INVALID_ID){ //passed.
        return 0;
    }
    status = scheduler_get_execution_count(blink_task_id, 0);
    if(status != SCHEDULER_ERR_INVALID_ARGUMENT){ //passed.
        return 0;
    }
    return 1;
}

int main(void)
{
const task_config_t counter_task_config = {
    .function = counter_task,
    .interval_ms = COUNTER_INTERVAL_MS,
    .policy = SCHEDULE_FIXED_RATE
};

const task_config_t blink_task_config = {
    .function = blink_task,
    .interval_ms = BLINK_INTERVAL_MS,
    .policy = SCHEDULE_FROM_LAST_RUN
};
task_id_t counter_task_id = TASK_ID_INVALID;
task_id_t blink_task_id = TASK_ID_INVALID;
gpio_init();

timer_init();
scheduler_status_t status;
scheduler_init(&status);

(void)scheduler_add_task(&counter_task_config, &counter_task_id);
if(status != SCHEDULER_OK)
{
    while(1){
        //Handle error
    }
}
(void)scheduler_add_task(&blink_task_config, &blink_task_id);
if(status != SCHEDULER_OK)
{
    while(1){
        //Handle error
    }
}

if(!scheduler_self_test(counter_task_id, blink_task_id))
{
    //keeps the blink led ON to indicate a failure in the self-test.
    PORTB |= (1U << BLINK_LED);
    while(1){
        //Handle error
    }
}

while (1)
{
    scheduler_run_once();
    uint32_t counter_executions;
    uint32_t blink_executions;
    (void)scheduler_get_execution_count(counter_task_id, &counter_executions);
    (void)scheduler_get_execution_count(blink_task_id, &blink_executions);
}

}
