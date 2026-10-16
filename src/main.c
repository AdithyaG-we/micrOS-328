#include <avr/io.h>
#include <stdint.h>

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

gpio_init();

timer_init();
scheduler_init();

(void)scheduler_add_task(&counter_task_config);
(void)scheduler_add_task(&blink_task_config);

while (1)
{
    scheduler_run_once();
}

}
