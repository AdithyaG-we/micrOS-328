#ifndef FUNCTIONS_H
#define FUNCTIONS_H
#include <avr/io.h>
#include "task.h"
#include "pwm.h"
#include "time.h"
#include "mutex.h"

#define COUNTER_BIT0 PB2
#define COUNTER_BIT1 PB3
#define COUNTER_BIT2 PB4
#define COUNTER_BIT3 PB5
#define ERROR_LED PB0
#define POWER_ON_LED PD4

#define COUNTER_MASK ((1U << COUNTER_BIT0) | \
                      (1U << COUNTER_BIT1) | \
                      (1U << COUNTER_BIT2) | \
                      (1U << COUNTER_BIT3))

#define BLINK_LED PB1
#define PWM_LED PD6

#define COUNTER_INTERVAL_MS MS_TO_TICKS(1000UL)
#define BLINK_INTERVAL_MS MS_TO_TICKS(300UL)
#define BRIGHTNESS_STEP 3
#define BRIGHTNESS_UPDATE_MS MS_TO_TICKS(25U)
#define ERROR_LED_TIME MS_TO_TICKS(1000UL)

extern uint8_t brightness;
extern int8_t brightness_step;

extern task_id_t counter_task_id;
extern task_id_t blink_task_id;
extern task_id_t breathe_task_id;

extern volatile uint8_t disable_breathing_requested;
extern volatile uint8_t disable_counter_requested;
extern volatile uint8_t disable_blinking_requested;

extern const task_config_t counter_task_config;
extern const task_config_t blink_task_config;
extern const task_config_t breathe_task_config;

void counter_task(void);
void led_breathe_task(void);
void blink_task(void);

void counter_task_cleanup(void);
void led_breathe_task_cleanup(void);
void blink_task_cleanup(void);

#endif