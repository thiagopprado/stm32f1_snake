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

typedef void (*timer_callback_t)(void);

typedef enum {
    TIMER_1 = 0,
    TIMER_2,
    TIMER_3,
    TIMER_NR,
} timer_idx_t;

void timer_setup(timer_idx_t timer, uint32_t psc, uint32_t arr);
void timer_attach_callback(timer_idx_t timer, timer_callback_t callback);

#endif /* TIMER_H */
