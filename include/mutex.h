#ifndef MUTEX_H
#define MUTEX_H

#include <stdint.h>
#include "task.h"

typedef struct{
    volatile uint8_t locked;
    task_id_t owner;
    uint8_t mask;
} mutex_t;

extern mutex_t portb_mutex;
extern mutex_t portc_mutex;
extern mutex_t portd_mutex;

void mutex_init(mutex_t* m);
uint8_t mutex_lock(mutex_t* m, task_id_t task_id);
void mutex_unlock(mutex_t* m);
mutex_t* determine_mutex(task_id_t id);

#endif