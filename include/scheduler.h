#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>
typedef uint8_t task_id_t;
#define SCHEDULER_MAX_TASKS 0x08
#define TASK_ID_INVALID 0x00

//task scheduling policies
typedef enum
{
SCHEDULE_FROM_LAST_RUN = 0,
SCHEDULE_FIXED_RATE
} schedule_policy_t;

// Scheduler status
typedef enum
{
SCHEDULER_OK = 0,
SCHEDULER_ERR_INVALID_TASK,
SCHEDULER_ERR_FULL,
SCHEDULER_ERR_INVALID_ID,
SCHEDULER_ERR_INVALID_ARGUMENT
} scheduler_status_t;

typedef struct
{
void (*function)(void);
uint32_t interval_ms;
schedule_policy_t policy;
} task_config_t;

void scheduler_init(scheduler_status_t *status);

scheduler_status_t scheduler_add_task(const task_config_t *config, task_id_t *id);

scheduler_status_t scheduler_get_execution_count(task_id_t id, uint32_t *count_out);

void scheduler_run_once(void);

void delete_one_task(task_id_t to_remove);

void update_next_task_id();

#endif
