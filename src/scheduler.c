#include <stdint.h>
#include "scheduler.h"
#include "timer.h"
#include <avr/io.h>

typedef struct
{
    void (*function)(void);
    uint8_t id;
    uint32_t interval_ms;
    uint32_t next_run;
    schedule_policy_t policy;
    uint32_t execution_count;
    uint8_t leds[MAX_LEDS];
    uint8_t led_count;
} task_entry_t;

task_entry_t tasks[SCHEDULER_MAX_TASKS];
static uint8_t task_count;
static uint8_t next_task_id;
void scheduler_init(scheduler_status_t *status)
{
    *status = SCHEDULER_OK;
    task_count = 0;
    next_task_id = 1; // conflict test point.
}

scheduler_status_t scheduler_add_task(const task_config_t *config, task_id_t *id)
{
    if (config == 0 ||
        config->function == 0 ||
        config->interval_ms == 0 || config->led_count > MAX_LEDS)
    {
        return SCHEDULER_ERR_INVALID_TASK;
    }
    if (id == 0)
    {
        return SCHEDULER_ERR_INVALID_TASK;
    }
    if (task_count >= SCHEDULER_MAX_TASKS)
    {
        return SCHEDULER_ERR_FULL;
    }
    // not a concern for now.
    if (next_task_id == TASK_ID_INVALID)
    {
        return SCHEDULER_ERR_FULL;
    }
    task_entry_t *task = &tasks[task_count];

    task->id = next_task_id;
    *id = next_task_id;
    next_task_id++; // conflict test point.
    task->function = config->function;
    task->interval_ms = config->interval_ms;
    task->execution_count = 0; // conflict test point.
    task->led_count = config->led_count;
    for (uint8_t i = 0; i < task->led_count; i++)
    {
        task->leds[i] = config->leds[i];
    }
    if (config->policy == SCHEDULE_FIXED_RATE)
    {
        task->policy = SCHEDULE_FIXED_RATE;
    }
    else
    {
        task->policy = SCHEDULE_FROM_LAST_RUN;
    }

    task->next_run = timer_get_ticks() + task->interval_ms;

    task_count++;

    return SCHEDULER_OK;
}

scheduler_status_t scheduler_get_execution_count(task_id_t id, uint32_t *count_out)
{
    if (count_out == 0)
    {
        return SCHEDULER_ERR_INVALID_ARGUMENT;
    }
    if (id == TASK_ID_INVALID)
    {
        return SCHEDULER_ERR_INVALID_ID;
    }
    for (uint8_t i = 0; i < task_count; i++)
    {
        if (tasks[i].id == id)
        {
            *count_out = tasks[i].execution_count;
            return SCHEDULER_OK;
        }
    }
    return SCHEDULER_ERR_INVALID_ID;
}

void scheduler_run_once(void)
{
    uint8_t i;

    for (i = 0; i < task_count; i++)
    {
        task_entry_t *task = &tasks[i];
        uint32_t now = timer_get_ticks();

        if ((int32_t)(now - task->next_run) >= 0)
        {
            task->function();
            task->execution_count++;

            if (task->policy == SCHEDULE_FIXED_RATE)
            {
                task->next_run += task->interval_ms;

                now = timer_get_ticks();

                if ((int32_t)(now - task->next_run) >= 0)
                {
                    task->next_run = now + task->interval_ms;
                }
            }
            else
            {
                task->next_run = timer_get_ticks() + task->interval_ms;
            }
        }
    }
}

// void update_next_task_id()
// {
//     uint8_t i = 0;
//     while (i < SCHEDULER_MAX_TASKS)
//     {
//         if (tasks[i].id == 0)
//         {
//             next_task_id = i + 1;
//             return;
//         }
//         i++;
//     }
// }

void delete_one_task(task_id_t to_remove)
{
    for (uint8_t i = 0; i < task_count; i++)
    {
        if (tasks[i].id == to_remove)
        {
            for(uint8_t j = 0; j < tasks[i].led_count; j++)
            {
                PORTB &= (uint8_t)~(1 << tasks[i].leds[j]); // turn off the LED
                tasks[i].leds[j] = 0;
            }
            // shift all subsequent tasks left by one
            for (uint8_t j = i; j < task_count - 1; j++)
            {
                tasks[j] = tasks[j + 1];
            }
            // zero out the now-vacant last slot
            tasks[task_count - 1].id = 0;
            tasks[task_count - 1].function = 0;
            tasks[task_count - 1].interval_ms = 0;
            tasks[task_count - 1].next_run = 0;
            tasks[task_count - 1].policy = SCHEDULE_FROM_LAST_RUN;
            tasks[task_count - 1].execution_count = 0;
            tasks[task_count - 1].led_count = 0;
            tasks[task_count - 1].leds[0] = 0;
            next_task_id = to_remove; // set next_task_id to the removed task's ID
            task_count--;
            return;
        }
    }
}
