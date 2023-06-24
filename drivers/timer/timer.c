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
timer_callback_t input_capture_callback[TIMER_NR][TIMER_CH_NR] = { NULL };

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
 * @brief Attach a callback function to timer ISR.
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
void timer_pwm_setup(timer_idx_t timer, timer_ch_t pwm_ch) {
    TIM_TypeDef *timer_ptr = timer_get_ptr(timer);

    if (timer_ptr == NULL || pwm_ch >= TIMER_CH_NR) {
        return;
    }

    timer_ptr->CR1 &= ~(TIM_CR1_CEN); // Disable counter

    timer_ptr->CR1 |= TIM_CR1_ARPE;

    switch (pwm_ch) {
        case TIMER_CH_1: {
            timer_ptr->CCMR1 |= TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1PE; // PWM mode 1
            timer_ptr->CCER |= TIM_CCER_CC1E;
            timer_ptr->CCR1 = 0;
            break;
        }
        case TIMER_CH_2: {
            timer_ptr->CCMR1 |= TIM_CCMR1_OC2M_2 | TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2PE; // PWM mode 1
            timer_ptr->CCER |= TIM_CCER_CC2E;
            timer_ptr->CCR2 = 0;
            break;
        }
        case TIMER_CH_3: {
            timer_ptr->CCMR2 |= TIM_CCMR2_OC3M_2 | TIM_CCMR2_OC3M_1 | TIM_CCMR2_OC3PE; // PWM mode 1
            timer_ptr->CCER |= TIM_CCER_CC3E;
            timer_ptr->CCR3 = 0;
            break;
        }
        case TIMER_CH_4: {
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
void timer_pwm_set_duty(timer_idx_t timer, timer_ch_t pwm_ch, uint32_t ccr) {
    TIM_TypeDef *timer_ptr = timer_get_ptr(timer);

    if (timer_ptr == NULL) {
        return;
    }

    switch (pwm_ch) {
        case TIMER_CH_1: {
            timer_ptr->CCR1 = ccr;
            break;
        }
        case TIMER_CH_2: {
            timer_ptr->CCR2 = ccr;
            break;
        }
        case TIMER_CH_3: {
            timer_ptr->CCR3 = ccr;
            break;
        }
        case TIMER_CH_4: {
            timer_ptr->CCR4 = ccr;
            break;
        }
        default: {
            break;
        }
    }
}

/**
 * @ingroup timer
 * @brief Sets up Input Capture.
 * 
 * @param timer             Timer index.
 * @param input_capture_ch  Input Capture channel.
 */
void timer_input_capture_setup(timer_idx_t timer, timer_ch_t input_capture_ch) {
    TIM_TypeDef *timer_ptr = timer_get_ptr(timer);

    if (timer_ptr == NULL || input_capture_ch >= TIMER_CH_NR) {
        return;
    }

    switch (input_capture_ch) {
        case TIMER_CH_1: {
            timer_ptr->CCMR1 |= TIM_CCMR1_CC1S_0;
            timer_ptr->CCER |= TIM_CCER_CC1E;
            break;
        }
        case TIMER_CH_2: {
            timer_ptr->CCMR1 |= TIM_CCMR1_CC2S_0;
            timer_ptr->CCER |= TIM_CCER_CC2E;
            break;
        }
        case TIMER_CH_3: {
            timer_ptr->CCMR2 |= TIM_CCMR2_CC3S_0;
            timer_ptr->CCER |= TIM_CCER_CC3E;
            break;
        }
        case TIMER_CH_4: {
            timer_ptr->CCMR2 |= TIM_CCMR2_CC4S_0;
            timer_ptr->CCER |= TIM_CCER_CC4E;
            break;
        }
        default: {
            break;
        }
    }


    timer_ptr->DIER |= TIM_DIER_CC1IE;
}

/**
 * @ingroup timer
 * @brief Attach a callback function to input capture ISR.
 */
void timer_attach_input_capture_callback(timer_idx_t timer, timer_ch_t input_capture_ch, timer_callback_t callback) {
    TIM_TypeDef *timer_ptr = timer_get_ptr(timer);
    IRQn_Type irqn = 0;

    switch (input_capture_ch) {
        case TIMER_CH_1: {
            timer_ptr->DIER |= TIM_DIER_CC1IE;
            break;
        }
        case TIMER_CH_2: {
            timer_ptr->DIER |= TIM_DIER_CC2IE;
            break;
        }
        case TIMER_CH_3: {
            timer_ptr->DIER |= TIM_DIER_CC3IE;
            break;
        }
        case TIMER_CH_4: {
            timer_ptr->DIER |= TIM_DIER_CC4IE;
            break;
        }
        default: {
            break;
        }
    }

    switch (timer) {
        case TIMER_1: {
            irqn = TIM1_UP_IRQn;
            break;
        }
        case TIMER_2: {
            irqn = TIM2_IRQn;
            break;
        }
        case TIMER_3: {
            irqn = TIM3_IRQn;
            break;
        }
        case TIMER_4: {
            irqn = TIM4_IRQn;
            break;
        }
        default: {
            return;
        }
    }

    NVIC_EnableIRQ(irqn);
    NVIC_SetPriority(irqn, 0);

    input_capture_callback[timer][input_capture_ch] = callback;
}

/**
 * @brief Inverts Input Capture polarity.
 * 
 * @param timer             Timer index.
 * @param input_capture_ch  Input Capture channel.
 */
void timer_invert_input_capture_polarity(timer_idx_t timer, timer_ch_t input_capture_ch)
{
    TIM_TypeDef *timer_ptr = timer_get_ptr(timer);

    if (timer_ptr == NULL || input_capture_ch >= TIMER_CH_NR) {
        return;
    }

    switch (input_capture_ch) {
        case TIMER_CH_1: {
            timer_ptr->CCER ^= TIM_CCER_CC1P;
            break;
        }
        case TIMER_CH_2: {
            timer_ptr->CCER ^= TIM_CCER_CC2P;
            break;
        }
        case TIMER_CH_3: {
            timer_ptr->CCER ^= TIM_CCER_CC3P;
            break;
        }
        case TIMER_CH_4: {
            timer_ptr->CCER ^= TIM_CCER_CC4P;
            break;
        }
        default: {
            break;
        }
    }
}

/**
 * @brief Gets the Input Capture counter.
 * 
 * @param timer             Timer index.
 * @param input_capture_ch  Input Capture channel.
 * 
 * @return Input Capture counter.
 */
uint16_t timer_get_input_capture_counter(timer_idx_t timer, timer_ch_t input_capture_ch)
{
    TIM_TypeDef *timer_ptr = timer_get_ptr(timer);
    uint16_t counter = 0;

    if (timer_ptr == NULL || input_capture_ch >= TIMER_CH_NR) {
        return counter;
    }

    switch (input_capture_ch) {
        case TIMER_CH_1: {
            counter = timer_ptr->CCR1;
            break;
        }
        case TIMER_CH_2: {
            counter = timer_ptr->CCR2;
            break;
        }
        case TIMER_CH_3: {
            counter = timer_ptr->CCR3;
            break;
        }
        case TIMER_CH_4: {
            counter = timer_ptr->CCR4;
            break;
        }
        default: {
            break;
        }
    }

    return counter;
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

    if ((TIM1->SR & TIM_SR_UIF) != 0) {
        // UIF bit (interrupt flag)
        TIM1->SR &= ~(TIM_SR_UIF);

        if (timer_callback[TIMER_1] != NULL) {
            timer_callback[TIMER_1]();
        }
    }

    if ((TIM1->SR & TIM_SR_CC1IF) != 0) {
        // CC1IF bit (interrupt flag)
        TIM1->SR &= ~(TIM_SR_CC1IF);

        if (input_capture_callback[TIMER_1][TIMER_CH_1] != NULL) {
            input_capture_callback[TIMER_1][TIMER_CH_1]();
        }
    }

    if ((TIM1->SR & TIM_SR_CC2IF) != 0) {
        // CC2IF bit (interrupt flag)
        TIM1->SR &= ~(TIM_SR_CC2IF);

        if (input_capture_callback[TIMER_1][TIMER_CH_2] != NULL) {
            input_capture_callback[TIMER_1][TIMER_CH_2]();
        }
    }

    if ((TIM1->SR & TIM_SR_CC3IF) != 0) {
        // CC3IF bit (interrupt flag)
        TIM1->SR &= ~(TIM_SR_CC3IF);

        if (input_capture_callback[TIMER_1][TIMER_CH_3] != NULL) {
            input_capture_callback[TIMER_1][TIMER_CH_3]();
        }
    }

    if ((TIM1->SR & TIM_SR_CC4IF) != 0) {
        // CC4IF bit (interrupt flag)
        TIM1->SR &= ~(TIM_SR_CC4IF);

        if (input_capture_callback[TIMER_1][TIMER_CH_4] != NULL) {
            input_capture_callback[TIMER_1][TIMER_CH_4]();
        }
    }
}

/**
 * @ingroup timer
 * @brief Timer 2 ISR.
 */
void TIM2_IRQHandler(void) {
    NVIC_ClearPendingIRQ(TIM2_IRQn);

    if ((TIM2->SR & TIM_SR_UIF) != 0) {
        // UIF bit (interrupt flag)
        TIM2->SR &= ~(TIM_SR_UIF);

        if (timer_callback[TIMER_2] != NULL) {
            timer_callback[TIMER_2]();
        }
    }

    if ((TIM2->SR & TIM_SR_CC1IF) != 0) {
        // CC1IF bit (interrupt flag)
        TIM2->SR &= ~(TIM_SR_CC1IF);

        if (input_capture_callback[TIMER_2][TIMER_CH_1] != NULL) {
            input_capture_callback[TIMER_2][TIMER_CH_1]();
        }
    }

    if ((TIM2->SR & TIM_SR_CC2IF) != 0) {
        // CC2IF bit (interrupt flag)
        TIM2->SR &= ~(TIM_SR_CC2IF);

        if (input_capture_callback[TIMER_2][TIMER_CH_2] != NULL) {
            input_capture_callback[TIMER_2][TIMER_CH_2]();
        }
    }

    if ((TIM2->SR & TIM_SR_CC3IF) != 0) {
        // CC3IF bit (interrupt flag)
        TIM2->SR &= ~(TIM_SR_CC3IF);

        if (input_capture_callback[TIMER_2][TIMER_CH_3] != NULL) {
            input_capture_callback[TIMER_2][TIMER_CH_3]();
        }
    }

    if ((TIM2->SR & TIM_SR_CC4IF) != 0) {
        // CC4IF bit (interrupt flag)
        TIM2->SR &= ~(TIM_SR_CC4IF);

        if (input_capture_callback[TIMER_2][TIMER_CH_4] != NULL) {
            input_capture_callback[TIMER_2][TIMER_CH_4]();
        }
    }
}

/**
 * @ingroup timer
 * @brief Timer 3 ISR.
 */
void TIM3_IRQHandler(void) {
    NVIC_ClearPendingIRQ(TIM3_IRQn);

    if ((TIM3->SR & TIM_SR_UIF) != 0) {
        // UIF bit (interrupt flag)
        TIM3->SR &= ~(TIM_SR_UIF);

        if (timer_callback[TIMER_3] != NULL) {
            timer_callback[TIMER_3]();
        }
    }

    if ((TIM3->SR & TIM_SR_CC1IF) != 0) {
        // CC1IF bit (interrupt flag)
        TIM3->SR &= ~(TIM_SR_CC1IF);

        if (input_capture_callback[TIMER_3][TIMER_CH_1] != NULL) {
            input_capture_callback[TIMER_3][TIMER_CH_1]();
        }
    }

    if ((TIM3->SR & TIM_SR_CC2IF) != 0) {
        // CC2IF bit (interrupt flag)
        TIM3->SR &= ~(TIM_SR_CC2IF);

        if (input_capture_callback[TIMER_3][TIMER_CH_2] != NULL) {
            input_capture_callback[TIMER_3][TIMER_CH_2]();
        }
    }

    if ((TIM3->SR & TIM_SR_CC3IF) != 0) {
        // CC3IF bit (interrupt flag)
        TIM3->SR &= ~(TIM_SR_CC3IF);

        if (input_capture_callback[TIMER_3][TIMER_CH_3] != NULL) {
            input_capture_callback[TIMER_3][TIMER_CH_3]();
        }
    }

    if ((TIM3->SR & TIM_SR_CC4IF) != 0) {
        // CC4IF bit (interrupt flag)
        TIM3->SR &= ~(TIM_SR_CC4IF);

        if (input_capture_callback[TIMER_3][TIMER_CH_4] != NULL) {
            input_capture_callback[TIMER_3][TIMER_CH_4]();
        }
    }
}


/**
 * @ingroup timer
 * @brief Timer 4 ISR.
 */
void TIM4_IRQHandler(void) {
    NVIC_ClearPendingIRQ(TIM4_IRQn);

    if ((TIM4->SR & TIM_SR_UIF) != 0) {
        // UIF bit (interrupt flag)
        TIM4->SR &= ~(TIM_SR_UIF);

        if (timer_callback[TIMER_4] != NULL) {
            timer_callback[TIMER_4]();
        }
    }

    if ((TIM4->SR & TIM_SR_CC1IF) != 0) {
        // CC1IF bit (interrupt flag)
        TIM4->SR &= ~(TIM_SR_CC1IF);

        if (input_capture_callback[TIMER_4][TIMER_CH_1] != NULL) {
            input_capture_callback[TIMER_4][TIMER_CH_1]();
        }
    }

    if ((TIM4->SR & TIM_SR_CC2IF) != 0) {
        // CC2IF bit (interrupt flag)
        TIM4->SR &= ~(TIM_SR_CC2IF);

        if (input_capture_callback[TIMER_4][TIMER_CH_2] != NULL) {
            input_capture_callback[TIMER_4][TIMER_CH_2]();
        }
    }

    if ((TIM4->SR & TIM_SR_CC3IF) != 0) {
        // CC3IF bit (interrupt flag)
        TIM4->SR &= ~(TIM_SR_CC3IF);

        if (input_capture_callback[TIMER_4][TIMER_CH_3] != NULL) {
            input_capture_callback[TIMER_4][TIMER_CH_3]();
        }
    }

    if ((TIM4->SR & TIM_SR_CC4IF) != 0) {
        // CC4IF bit (interrupt flag)
        TIM4->SR &= ~(TIM_SR_CC4IF);

        if (input_capture_callback[TIMER_4][TIMER_CH_4] != NULL) {
            input_capture_callback[TIMER_4][TIMER_CH_4]();
        }
    }
}
