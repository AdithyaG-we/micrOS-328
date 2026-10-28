#ifndef LED_PATTERN_H
#define LED_PATTERN_H
#include "task.h"
/* One cycle means one scheduler update. */
#define LED_PATTERN_STEP_MS 250UL
#define LED_PATTERN_STEPS 16U
extern task_id_t led_pattern_task_id;
extern const task_config_t led_pattern_task_config;
void led_pattern_init(void);
void led_pattern_task(void);
void led_pattern_cleanup(void);
#endif
