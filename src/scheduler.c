#include <stdint.h>
#include "scheduler.h"
#include "timer.h"
#include "task.h"
#include <avr/io.h>
#include <stddef.h>

void scheduler_init(scheduler_status_t *status)
{
    *status = SCHEDULER_OK;
    set_task_count(0);
    set_next_task_id(1); // conflict test point.
}

uint8_t scheduler_sanity_check(void)
{
    uint32_t count;
    scheduler_status_t status;
    // ensure next task ID is set correctly after initialization.
    if (get_next_task_id() != 1)
    {
        return 0;
    }
    // ensure execution counts are zero at initialisation.
    for (uint8_t i = 1; i <= MAX_TASKS; i++)
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
    status = scheduler_get_execution_count(MAX_TASKS + 1, &count);
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

void scheduler_run_once(void)
{
    uint8_t i;
    uint8_t task_count = get_task_count();
    for (i = 0; i < task_count; i++)
    {
        task_entry_t *task = get_task_by_index(i);
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

scheduler_status_t add_and_verify_task(const task_config_t *config, task_id_t *id)
{
    if (config->function == NULL || config->interval_ms == 0)
    {
        return SCHEDULER_ERR_INVALID_ARGUMENT;
    }
    scheduler_status_t status = add_task(config, id);
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
