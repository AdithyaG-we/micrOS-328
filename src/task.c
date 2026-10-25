#include "task.h"
#include <avr/io.h>
#include "time.h"
#include <stddef.h>

static uint8_t task_count = 0;
static uint8_t next_task_id = 1;
static task_entry_t tasks[MAX_TASKS];

scheduler_status_t add_task(const task_config_t *config, task_id_t *id)
{
    if (config == 0 ||
        config->function == 0 ||
        config->interval_ms == 0 || config->led_count > MAX_LEDS)
    {
        return SCHEDULER_ERR_INVALID_ARGUMENT;
    }
    if (id == 0)
    {
        return SCHEDULER_ERR_INVALID_ARGUMENT;
    }
    if (task_count >= MAX_TASKS)
    {
        return SCHEDULER_ERR_FULL;
    }
    // not a concern for now.
    if (next_task_id == TASK_ID_INVALID)
    {
        return SCHEDULER_ERR_FULL;
    }
    task_entry_t *task = &tasks[task_count];
    task_count++;
    task->id = next_task_id;
    *id = next_task_id;
    update_next_task_id(); // conflict test point.
    task->function = config->function;
    task->cleanup = config->cleanup;
    task->interval_ms = config->interval_ms;
    task->state = config->state;
    task->execution_count = 0; // conflict test point.
    task->led_count = config->led_count;
    task->port = config->port;
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

    task->next_run = timer_get_hardware_ticks() + task->interval_ms;

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

void update_next_task_id(void)
{
    for (uint8_t candidate = 1; candidate != TASK_ID_INVALID; candidate++)
    {
        uint8_t in_use = 0;
        for (uint8_t i = 0; i < task_count; i++)
        {
            if (tasks[i].id == candidate)
            {
                in_use = 1;
                break;
            }
        }
        if (!in_use)
        {
            next_task_id = candidate;
            return;
        }
    }
    next_task_id = TASK_ID_INVALID; // all IDs exhausted.
}

void delete_one_task(task_id_t to_remove)
{
    for (uint8_t i = 0; i < task_count; i++)
    {
        if (tasks[i].id == to_remove)
        {
            if (tasks[i].cleanup != NULL)
            {
                tasks[i].cleanup();
            }
            // shift all subsequent tasks left by one unit.
            for (uint8_t j = i; j < task_count - 1; j++)
            {
                tasks[j] = tasks[j + 1];
            }
            // zero out the now-vacant last slot
            tasks[task_count - 1].id = 0;
            tasks[task_count - 1].function = 0;
            tasks[task_count - 1].cleanup = 0;
            tasks[task_count - 1].interval_ms = 0;
            tasks[task_count - 1].next_run = 0;
            tasks[task_count - 1].state = TASK_BLOCKED;
            tasks[task_count - 1].policy = SCHEDULE_FROM_LAST_RUN;
            tasks[task_count - 1].execution_count = 0;
            tasks[task_count - 1].led_count = 0;
            tasks[task_count - 1].port = Z; // default to invalid port.
            task_count--;
            update_next_task_id(); // set next_task_id to the removed task's ID
            return;
        }
    }
}

uint8_t get_next_task_id(void)
{
    return next_task_id;
}

uint8_t get_task_count(void){
    return task_count;
}


void set_next_task_id(uint8_t id){
    next_task_id = id;
}
void set_task_count(uint8_t count){
    task_count = count;
}
task_entry_t* get_task_by_index(uint8_t index){
    if(index < task_count){
        return &tasks[index];
    }
    return NULL;
}

task_entry_t* get_task_by_id(uint8_t id){
    uint8_t i = 0;
    for(i = 0; i < task_count; i++){
        if(tasks[i].id == id){
            return &tasks[i];
        }
    }
    return NULL;
}
