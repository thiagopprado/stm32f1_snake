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

#include "core_cm3.h"

timer_callback_t timer_callback[TIMER_NR] = { NULL };

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
        case TIMER_4: {
            RCC->APB1ENR |= RCC_APB1ENR_TIM4EN; // Enable Timer 4 clock
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
 * @brief Updates timer prescaler (frequency).
 * 
 * @param timer     Timer index.
 * @param psc       Prescaler.
 * @param arr       Autoreload value.
 */
void timer_update_psc(timer_idx_t timer, uint32_t psc, uint32_t arr) {
    TIM_TypeDef *timer_ptr = timer_get_ptr(timer);

    timer_ptr->PSC = psc;
    timer_ptr->ARR = arr;
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
        case TIMER_4: {
            RCC->APB1ENR |= RCC_APB1ENR_TIM4EN; // Enable Timer 4 clock
            irqn = TIM4_IRQn;
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
 * @brief Sets up PWM.
 * 
 * @param timer     Timer index.
 * @param pwm_ch    PWM channel.
 */
void timer_pwm_setup(timer_idx_t timer, timer_pwm_ch_t pwm_ch) {
    TIM_TypeDef *timer_ptr = timer_get_ptr(timer);

    if (timer_ptr == NULL || pwm_ch >= TIMER_PWM_CH_NR) {
        return;
    }

    timer_ptr->CR1 &= ~(TIM_CR1_CEN); // Disable counter

    timer_ptr->CR1 |= TIM_CR1_ARPE;

    switch (pwm_ch) {
        case TIMER_PWM_CH_1: {
            timer_ptr->CCMR1 |= TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1PE; // PWM mode 1
            timer_ptr->CCER |= TIM_CCER_CC1E;
            timer_ptr->CCR1 = 0;
            break;
        }
        case TIMER_PWM_CH_2: {
            timer_ptr->CCMR1 |= TIM_CCMR1_OC2M_2 | TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2PE; // PWM mode 1
            timer_ptr->CCER |= TIM_CCER_CC2E;
            timer_ptr->CCR2 = 0;
            break;
        }
        case TIMER_PWM_CH_3: {
            timer_ptr->CCMR2 |= TIM_CCMR2_OC3M_2 | TIM_CCMR2_OC3M_1 | TIM_CCMR2_OC3PE; // PWM mode 1
            timer_ptr->CCER |= TIM_CCER_CC3E;
            timer_ptr->CCR3 = 0;
            break;
        }
        case TIMER_PWM_CH_4: {
            timer_ptr->CCMR2 |= TIM_CCMR2_OC4M_2 | TIM_CCMR2_OC4M_1 | TIM_CCMR2_OC4PE; // PWM mode 1
            timer_ptr->CCER |= TIM_CCER_CC4E;
            timer_ptr->CCR4 = 0;
            break;
        }
        default: {
            break;
        }
    }

    timer_ptr->EGR |= TIM_EGR_UG;
    timer_ptr->CR1 |= TIM_CR1_CEN; // Enable counter
}

/**
 * @brief Sets PWM duty cycle.
 * 
 * @param timer     Timer index.
 * @param pwm_ch    PWM channel.
 * @param ccr       CCR value.
 * 
 * Duty cycle is defined as CCR / ARR.
 */
void timer_pwm_set_duty(timer_idx_t timer, timer_pwm_ch_t pwm_ch, uint32_t ccr) {
    TIM_TypeDef *timer_ptr = timer_get_ptr(timer);

    if (timer_ptr == NULL) {
        return;
    }

    switch (pwm_ch) {
        case TIMER_PWM_CH_1: {
            timer_ptr->CCR1 = ccr;
            break;
        }
        case TIMER_PWM_CH_2: {
            timer_ptr->CCR2 = ccr;
            break;
        }
        case TIMER_PWM_CH_3: {
            timer_ptr->CCR3 = ccr;
            break;
        }
        case TIMER_PWM_CH_4: {
            timer_ptr->CCR4 = ccr;
            break;
        }
        default: {
            break;
        }
    }
}

/**
 * @brief Gets the timer structure pointer.
 * 
 * @param timer     Timer index.
 * 
 * @return Pointer for the timer structure.
 */
TIM_TypeDef* timer_get_ptr(timer_idx_t timer) {
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
        case TIMER_4: {
            timer_ptr = TIM4;
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
void TIM2_IRQHandler(void) {
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
void TIM3_IRQHandler(void) {
    NVIC_ClearPendingIRQ(TIM3_IRQn);

    TIM3->SR &= ~(TIM_SR_UIF); // UIF bit (interrupt flag)

    if (timer_callback[TIMER_3] != NULL) {
        timer_callback[TIMER_3]();
    }
}


/**
 * @ingroup timer
 * @brief Timer 4 ISR.
 */
void TIM4_IRQHandler(void) {
    NVIC_ClearPendingIRQ(TIM4_IRQn);

    TIM4->SR &= ~(TIM_SR_UIF); // UIF bit (interrupt flag)

    if (timer_callback[TIMER_4] != NULL) {
        timer_callback[TIMER_4]();
    }
}
