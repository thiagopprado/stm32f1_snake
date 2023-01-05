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

timer_callback_t timer_callback[TIMER_NR] = { NULL };

static TIM_TypeDef* timer_get_ptr(timer_idx_t timer);

static TIM_TypeDef* timer_get_ptr(timer_idx_t timer) {
    TIM_TypeDef *timer_ptr = NULL;

    switch (timer) {
        case TIMER_1: {
            timer_ptr = TIM1;
            break;
        }
        case TIMER_2: {
            timer_ptr = TIM2;
            break;
        }
        case TIMER_3: {
            timer_ptr = TIM3;
            break;
        }
        default: {
            break;
        }
    }

    return timer_ptr;
}

/**
 * @ingroup timer
 * @brief Sets up Timer.
 * 
 * @param timer Timer index.
 * @param psc   Timer prescaler.
 * @param arr   Timer autoreload.
 */
void timer_setup(timer_idx_t timer, uint32_t psc, uint32_t arr) {
    TIM_TypeDef *timer_ptr = timer_get_ptr(timer);

    switch (timer) {
        case TIMER_1: {
            RCC->APB2ENR |= RCC_APB2ENR_TIM1EN; // Enable Timer 1 clock
            break;
        }
        case TIMER_2: {
            RCC->APB1ENR |= RCC_APB1ENR_TIM2EN; // Enable Timer 2 clock
            break;
        }
        case TIMER_3: {
            RCC->APB1ENR |= RCC_APB1ENR_TIM3EN; // Enable Timer 3 clock
            break;
        }
        default: {
            return;
        }
    }

    timer_ptr->PSC = psc; // Prescaler
    timer_ptr->ARR = arr; // Counter overflow value

    timer_ptr->CR1 |= TIM_CR1_CEN; // Enable counter
}

/**
 * @ingroup timer
 * @brief Attach a callback function to timer 1 ISR.
 */
void timer_attach_callback(timer_idx_t timer, timer_callback_t callback) {
    TIM_TypeDef *timer_ptr = timer_get_ptr(timer);
    IRQn_Type irqn = 0;

    switch (timer) {
        case TIMER_1: {
            RCC->APB2ENR |= RCC_APB2ENR_TIM1EN; // Enable Timer 1 clock
            irqn = TIM1_UP_IRQn;
            break;
        }
        case TIMER_2: {
            RCC->APB1ENR |= RCC_APB1ENR_TIM2EN; // Enable Timer 2 clock
            irqn = TIM2_IRQn;
            break;
        }
        case TIMER_3: {
            RCC->APB1ENR |= RCC_APB1ENR_TIM3EN; // Enable Timer 3 clock
            irqn = TIM3_IRQn;
            break;
        }
        default: {
            return;
        }
    }

    timer_ptr->DIER |= TIM_DIER_UIE; // Enable timer update event interrupt

    NVIC_EnableIRQ(irqn);
    NVIC_SetPriority(irqn, 0);

    timer_callback[timer] = callback;
}

/**
 * @ingroup timer
 * @brief Timer 1 ISR.
 */
void TIM1_UP_IRQHandler(void) {
    NVIC_ClearPendingIRQ(TIM1_UP_IRQn);

    TIM1->SR &= ~(TIM_SR_UIF); // UIF bit (interrupt flag)

    if (timer_callback[TIMER_1] != NULL) {
        timer_callback[TIMER_1]();
    }
}

/**
 * @ingroup timer
 * @brief Timer 2 ISR.
 */
void TIM2_UP_IRQHandler(void) {
    NVIC_ClearPendingIRQ(TIM2_IRQn);

    TIM2->SR &= ~(TIM_SR_UIF); // UIF bit (interrupt flag)

    if (timer_callback[TIMER_2] != NULL) {
        timer_callback[TIMER_2]();
    }
}

/**
 * @ingroup timer
 * @brief Timer 3 ISR.
 */
void TIM3_UP_IRQHandler(void) {
    NVIC_ClearPendingIRQ(TIM3_IRQn);

    TIM3->SR &= ~(TIM_SR_UIF); // UIF bit (interrupt flag)

    if (timer_callback[TIMER_3] != NULL) {
        timer_callback[TIMER_3]();
    }
}
