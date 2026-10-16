#include <stdint.h>
#include "scheduler.h"
#include "timer.h"

typedef struct
{
void (*function)(void);
uint32_t interval_ms;
uint32_t next_run;
schedule_policy_t policy;
} task_entry_t;

static task_entry_t tasks[SCHEDULER_MAX_TASKS];
static uint8_t task_count;

void scheduler_init(void)
{
task_count = 0;
}

scheduler_status_t scheduler_add_task(const task_config_t *config)
{
if (config == 0 ||
config->function == 0 ||
config->interval_ms == 0)
{
return SCHEDULER_ERR_INVALID_TASK;
}

if (task_count >= SCHEDULER_MAX_TASKS)
{
    return SCHEDULER_ERR_FULL;
}

task_entry_t *task = &tasks[task_count];

task->function = config->function;
task->interval_ms = config->interval_ms;

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
