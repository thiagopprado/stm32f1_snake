/**
 * Author: thiagopereiraprado@gmail.com
 * 
 * @brief Infrared module implementation.
 * 
 */
#include "led_ws2812.h"

#include <stddef.h>

#include "timer.h"

#include "stm32f1xx.h"
#include "core_cm3.h"

#define BIT_CHECK(value, bit)       ((value >> bit) & 0x01)
#define BIT_SET(value, bit)         (value |= 1 << bit)
#define BIT_CLEAR(value, bit)       (value &= ~(1 << bit))

#define LED_WS2812_DUTY_CYCLE_BIT_0     30
#define LED_WS2812_DUTY_CYCLE_BIT_1     50
#define LED_WS2812_BITS_NR              24

/**
 * @brief PWM prescaler.
 * 
 * This generates a 800khz frequency.
 * @{
 */
#define LED_WS2812_PSC  0
#define LED_WS2812_ARR  89
/** @} */

/**
 * @brief PWM buffer size.
 * 
 * Each bit represents a PWM duty cycle value. PWM buffer has the same positions number
 * as the number of bits needed to update all LEDs plus one. The final duty cycle is always
 * zero, to stop sending new bits.
 */
#define LED_WS2812_PWM_BUFFER_SZ    ((LED_WS2812_BITS_NR * LED_WS2812_NR) + 1)    

static uint16_t led_pwm_buffer[LED_WS2812_PWM_BUFFER_SZ] = { 0 };

/**
 * @brief Waits for DMA to finish the current transmission;
 */
static void led_ws2812_pwm_wait_transmission(void) {
    while ((DMA1_Channel3->CCR & DMA_CCR_EN) != 0);
}

/**
 * @brief Starts the next DMA transmission;
 * 
 * @param trx_len   Transmission length.
 */
static void led_ws2812_pwm_init_transmission(uint32_t trx_len) {
    TIM_TypeDef *timer_ptr = timer_get_ptr(LED_WS2812_TIMER);

    if (timer_ptr == NULL) {
        return;
    }

    timer_ptr->EGR |= TIM_EGR_UG;

    DMA1_Channel3->CNDTR = trx_len;
    DMA1_Channel3->CCR |= DMA_CCR_EN;
}

/**
 * @brief Sets up LED ws2812 driver.
 * 
 * The communication is done varying the PWM duty cycle at a fixed frequency.
 * The duty cycle is updated via DMA.
 */
void led_ws2812_setup(void) {
    TIM_TypeDef *timer_ptr = timer_get_ptr(LED_WS2812_TIMER);

    if (timer_ptr == NULL) {
        return;
    }

    gpio_setup(LED_WS2812_PORT, LED_WS2812_PIN, GPIO_MODE_OUTPUT_50, GPIO_CFG_OUT_AF_OPEN_DRAIN);
    timer_setup(LED_WS2812_TIMER, LED_WS2812_PSC, LED_WS2812_ARR);
    timer_pwm_setup(LED_WS2812_TIMER, LED_WS2812_PWM_CH);
    
    // DMA1
    RCC->AHBENR |= RCC_AHBENR_DMA1EN;
    DMA1_Channel3->CCR |= DMA_CCR_MSIZE_0 | DMA_CCR_PSIZE_0 | DMA_CCR_MINC | DMA_CCR_DIR | DMA_CCR_TCIE;
    DMA1_Channel3->CMAR = (uint32_t)led_pwm_buffer;

    // Enable compare channel DMA request
    switch (LED_WS2812_PWM_CH) {
        case TIMER_CH_1: {
            timer_ptr->DIER |= TIM_DIER_CC1DE;
            DMA1_Channel3->CPAR = (uint32_t)&(timer_ptr->CCR1);
            break;
        }
        case TIMER_CH_2: {
            timer_ptr->DIER |= TIM_DIER_CC2DE;
            DMA1_Channel3->CPAR = (uint32_t)&(timer_ptr->CCR2);
            break;
        }
        case TIMER_CH_3: {
            timer_ptr->DIER |= TIM_DIER_CC3DE;
            DMA1_Channel3->CPAR = (uint32_t)&(timer_ptr->CCR3);
            break;
        }
        case TIMER_CH_4: {
            timer_ptr->DIER |= TIM_DIER_CC4DE;
            DMA1_Channel3->CPAR = (uint32_t)&(timer_ptr->CCR4);
            break;
        }
        default: {
            break;
        }
    }

    NVIC_EnableIRQ(DMA1_Channel3_IRQn);
    NVIC_SetPriority(DMA1_Channel3_IRQn, 0);
}

/**
 * @brief Updates the LEDs.
 * 
 * @param color     Color buffer.
 * @param led_nr    Number of LEDs to be updated.
 */
void led_ws2812_write(uint32_t *color, uint16_t led_nr) {
    uint16_t led_idx = 0;
    uint32_t pwm_idx = 0;

    led_ws2812_pwm_wait_transmission();

    for (led_idx = 0; led_idx < led_nr; led_idx++) {
        int32_t i;

        for (i = (LED_WS2812_BITS_NR - 1); i >= 0; i--) {
            if (BIT_CHECK(color[led_idx], i) == 0) {
                led_pwm_buffer[pwm_idx++] = LED_WS2812_DUTY_CYCLE_BIT_0;
            } else {
                led_pwm_buffer[pwm_idx++] = LED_WS2812_DUTY_CYCLE_BIT_1;
            }
        }
    }

    led_pwm_buffer[pwm_idx++] = 0;

    // Init transmission
    led_ws2812_pwm_init_transmission(pwm_idx);
}

void DMA1_Channel3_IRQHandler(void) {
    NVIC_ClearPendingIRQ(DMA1_Channel3_IRQn);

    DMA1->IFCR |= DMA_IFCR_CGIF3;
    DMA1_Channel3->CCR &= ~(DMA_CCR_EN);
}
