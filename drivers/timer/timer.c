/**
 * Author: thiagopereiraprado@gmail.com
 * 
 * @file
 * @ingroup timer
 * @brief STM32F1 timer driver
 * 
 */
#include "timer.h"

#include <stddef.h>

#include "stm32f1xx.h"
#include "core_cm3.h"

timer_callback_t timer_callback = NULL;

/**
 * @ingroup timer
 * @brief Sets up Timer 1.
 * 
 * @param time  Timer period in tenths of ms.
 */
void timer_setup(uint32_t time) {
    RCC->APB2ENR |= RCC_APB2ENR_TIM1EN; // Enable Timer 1 clock

    TIM1->PSC = 7200; // Prescaler
    TIM1->ARR = time; // Counter overflow value

    TIM1->DIER |= TIM_DIER_UIE; // Enable timer update event interrupt

    NVIC_EnableIRQ(TIM1_UP_IRQn);
    NVIC_SetPriority(TIM1_UP_IRQn, 0);

    TIM1->CR1 |= TIM_CR1_CEN; // Enable counter
}

/**
 * @ingroup timer
 * @brief Attach a callback function to timer 1 ISR.
 */
void timer_attach_callback(timer_callback_t callback) {
    timer_callback = callback;
}

/**
 * @ingroup timer
 * @brief Timer 1 ISR.
 */
void TIM1_UP_IRQHandler(void) {
    NVIC_ClearPendingIRQ(TIM1_UP_IRQn);

    TIM1->SR &= ~(TIM_SR_UIF); // UIF bit (interrupt flag)

    if (timer_callback != NULL) {
        timer_callback();
    }
}
