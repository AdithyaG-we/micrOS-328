#include "mutex.h"
#include <util/atomic.h>
#include "functions.h"

void mutex_init(mutex_t *m)
{
    m->locked = 0;
    m->owner = TASK_ID_INVALID;
    m->mask = 0x00;
    m->initial_priority = 0;
}

uint8_t mutex_lock(mutex_t *m, task_id_t task_id)
{
    uint8_t acquired = 0;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        if (!m->locked)
        {
            task_entry_t *t = get_task_by_id(task_id);
            if (t != NULL)
            {
                m->locked = 1;
                m->owner = task_id;
                m->initial_priority = t->priority;
                acquired = 1;
            }
        }
        else
        {
            task_entry_t *t = get_task_by_id(task_id);
            if (t != NULL)
            {
                t->state = TASK_BLOCKED;
                m->mask |= (1U << (t->id - 1));
                task_entry_t *present = get_task_by_id(m->owner);
                if (present != NULL && present->priority < t->priority)
                {
                    present->priority = t->priority;
                }
            }
        }
    }
    return acquired;
}

void mutex_unlock(mutex_t *m)
{
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        m->locked = 0;
        task_entry_t *initial = get_task_by_id(m->owner);
        if (initial != NULL)
            initial->priority = m->initial_priority;
        m->owner = TASK_ID_INVALID;
        m->initial_priority = 0;
        if (m->mask)
        {
            for (int i = 0; i < 8; i++)
            {
                if (m->mask & (1U << i))
                {
                    task_entry_t *t = get_task_by_id(i + 1);
                    if(t != NULL) t->state = TASK_READY;
                }
            }
            m->mask = 0x00;
        }
    }
}

mutex_t *determine_mutex(task_id_t id)
{
    task_entry_t *t = get_task_by_id(id);
    if (t == NULL)
        return NULL;
    if (t->port == B)
    {
        return &portb_mutex;
    }
    else if (t->port == C)
    {
        return &portc_mutex;
    }
    else if (t->port == D)
    {
        return &portd_mutex;
    }
    return NULL;
}