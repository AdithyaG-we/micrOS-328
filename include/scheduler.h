#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>

#define SCHEDULER_MAX_TASKS 8

// Task scheduling policies
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
SCHEDULER_ERR_FULL
} scheduler_status_t;

typedef struct
{
void (*function)(void);
uint32_t interval_ms;
schedule_policy_t policy;
} task_config_t;

void scheduler_init(void);

scheduler_status_t scheduler_add_task(const task_config_t *config);

void scheduler_run_once(void);

#endif
