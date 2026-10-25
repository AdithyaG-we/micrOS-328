#include "mutex.h"
#include <util/atomic.h>
#include "functions.h"

void mutex_init(mutex_t *m)
{
    m->locked = 0;
    m->owner = TASK_ID_INVALID;
    m->waiting_task_id = TASK_ID_INVALID;
}

uint8_t mutex_lock(mutex_t *m, task_id_t task_id)
{
    uint8_t acquired = 0;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        if (!m->locked)
        {
            m->locked = 1;
            m->owner = task_id;
            acquired = 1;
        }
        else
        {
            task_entry_t *t = get_task_by_id(task_id);
            if (t != NULL)
            {
                t->state = TASK_BLOCKED;
            }
            m->waiting_task_id = task_id;
        }
    }
    return acquired;
}

void mutex_unlock(mutex_t *m)
{
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        m->locked = 0;
        m->owner = TASK_ID_INVALID;
        if (m->waiting_task_id != TASK_ID_INVALID)
        {
            task_entry_t *t = get_task_by_id(m->waiting_task_id);
            if (t != NULL)
            {
                t->state = TASK_READY;
            }
            m->waiting_task_id = TASK_ID_INVALID;
        }
    }
}

mutex_t* determine_mutex(task_id_t id){
    task_entry_t* t = get_task_by_id(id);
    if(t == NULL) return NULL;
    if(t->port == B){
        return &portb_mutex;
    }
    else if(t->port == C){
        return &portc_mutex;
    }
    else if(t->port == D){
        return &portd_mutex;
    }
    return NULL;
}