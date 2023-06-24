/**
 * Author: thiagopereiraprado@gmail.com
 * 
 * @file
 * @defgroup timer STM32F1 timer driver
 * @brief STM32F1 timer driver
 * 
 */
#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

#include "stm32f1xx.h"

typedef void (*timer_callback_t)(void);

typedef enum {
    TIMER_1 = 0,
    TIMER_2,
    TIMER_3,
    TIMER_4,
    TIMER_NR,
} timer_idx_t;

typedef enum {
    TIMER_CH_1 = 0,
    TIMER_CH_2,
    TIMER_CH_3,
    TIMER_CH_4,
    TIMER_CH_NR,
} timer_ch_t;

void timer_setup(timer_idx_t timer, uint32_t psc, uint32_t arr);
void timer_update_psc(timer_idx_t timer, uint32_t psc, uint32_t arr);
void timer_attach_callback(timer_idx_t timer, timer_callback_t callback);
void timer_pwm_setup(timer_idx_t timer, timer_ch_t pwm_ch);
void timer_pwm_set_duty(timer_idx_t timer, timer_ch_t pwm_ch, uint32_t ccr);
void timer_input_capture_setup(timer_idx_t timer, timer_ch_t input_capture_ch);
void timer_attach_input_capture_callback(timer_idx_t timer, timer_ch_t input_capture_ch, timer_callback_t callback);
void timer_invert_input_capture_polarity(timer_idx_t timer, timer_ch_t input_capture_ch);
uint16_t timer_get_input_capture_counter(timer_idx_t timer, timer_ch_t input_capture_ch);

TIM_TypeDef* timer_get_ptr(timer_idx_t timer);

#endif /* TIMER_H */
