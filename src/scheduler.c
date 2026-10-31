#include <stdint.h>
#include "scheduler.h"
#include "time.h"
#include "task.h"
#include <avr/io.h>
#include <stddef.h>
#include "mutex.h"

void scheduler_init(scheduler_status_t *status)
{
    set_task_count(0);
    set_next_task_id(1); // conflict test point.
    *status = SCHEDULER_OK;
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

// void scheduler_run_once(void)
// {
//     uint8_t i;
//     uint8_t task_count = get_task_count();
//     for (i = 0; i < task_count; i++)
//     {
//         task_entry_t *task = get_task_by_index(i);
//         if(task->state != TASK_READY){
//             continue;
//         }
//         uint32_t now = timer_get_hardware_ticks();

//         if ((int32_t)(now - task->next_run) >= 0)
//         {
//             mutex_t* m = determine_mutex(task->id);
//             if(m != NULL && !mutex_lock(m, task->id)) continue;
//             task->state = TASK_RUNNING;
//             task->function();
//             task->state = TASK_READY;
//             task->execution_count++;

//             if (task->policy == SCHEDULE_FIXED_RATE)
//             {
//                 task->next_run += task->interval_ms;

//                 now = timer_get_hardware_ticks();

//                 if ((int32_t)(now - task->next_run) >= 0)
//                 {
//                     task->next_run = now + task->interval_ms;
//                 }
//             }
//             else
//             {
//                 task->next_run = timer_get_hardware_ticks() + task->interval_ms;
//             }
//             mutex_unlock(m);
//         }
//     }
// }

//priority scheduler:
void scheduler_run_once(){
    uint8_t task_count = get_task_count();
    uint32_t now = timer_get_hardware_ticks();
    task_entry_t* best_task = NULL;
    uint8_t highest_priority = 0;

    for(uint8_t i = 0; i < task_count; i++){
        task_entry_t* task = get_task_by_index(i);
        if(task == NULL || task->state != TASK_READY) continue;

        if(best_task == NULL || task->priority > highest_priority){
            best_task = task;
            highest_priority = task->priority;
        }
    }

    //we found the best task, now run that task normally.
    if(best_task != NULL){
        mutex_t* m = determine_mutex(best_task->id);
        if(m != NULL && m->owner != best_task->id && !mutex_lock(m, best_task->id)) return;

        best_task->state = TASK_RUNNING;
        best_task->function();
        if(best_task->state == TASK_RUNNING) best_task->state = TASK_WAITING;
        best_task->execution_count++;

        if(best_task->policy == SCHEDULE_FIXED_RATE){
            best_task->next_run += best_task->interval_ms;
            now = timer_get_hardware_ticks();
            if((int32_t)(now - best_task->next_run) >= 0){
                best_task->next_run = now + best_task->interval_ms;
            }
        }
        else{
            best_task->next_run = timer_get_hardware_ticks() + best_task->interval_ms;
        }
        if(m != NULL && !best_task->multi_stage){
            mutex_unlock(m);
        }
    }
}

scheduler_status_t add_and_verify_task(const task_config_t *config, task_id_t *id)
{
    if (config->function == NULL)
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

void update_ready_list(){
    uint8_t i = 0;
    uint8_t task_count = get_task_count();
    for(i = 0; i < task_count; i++){
        task_entry_t* t = get_task_by_index(i);
        if(t->state != TASK_WAITING) continue;
        if((int32_t)(timer_get_hardware_ticks() - t->next_run) >= 0){
            t->state = TASK_READY;
        }
    }
}

uint32_t determine_wake_up_time(){
    uint32_t wake_up_time = 0xFFFFFFFF;
    uint8_t task_count = get_task_count();
    uint8_t waiting = 0;
    if(!task_count) return timer_get_hardware_ticks();

    for(uint8_t i = 0; i < task_count; i++){
        task_entry_t* t = get_task_by_index(i);
        if(t == NULL) continue;
        if(t->state == TASK_READY) return timer_get_hardware_ticks();
        else if(t->state == TASK_BLOCKED) continue;
        if(t->state == TASK_WAITING && (int64_t)(wake_up_time - t->next_run) > 0){
            wake_up_time = t->next_run;
            waiting = 1;
        }
    }
    if(!waiting){
        return timer_get_hardware_ticks();
    }
    return wake_up_time;
}
