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

void timer_setup(uint32_t time);
void timer_attach_callback(timer_callback_t callback);

#endif /* TIMER_H */
