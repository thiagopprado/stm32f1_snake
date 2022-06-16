/**
 * Author: thiagopereiraprado@gmail.com
 * 
 * @file
 * @ingroup rcc
 * @brief STM32F1 RCC driver
 * 
 */
#include "rcc.h"

#include "stm32f1xx.h"

/**
 * @ingroup rcc
 * @brief Sets up the MCU clock.
 *
 * Configures HSE (High Speed External oscilator) and PLL, with 9x multiplier.
 * Sets up PLL as SYSCLK.
 */
void rcc_clock_init(void) {
    // Enable HSE
    RCC->CR |= RCC_CR_HSEON;
    while ((RCC->CR & RCC_CR_HSERDY) == 0);

    // Flash latency
    FLASH->ACR |= FLASH_ACR_PRFTBE;
    FLASH->ACR &= ~(FLASH_ACR_LATENCY);
    FLASH->ACR |= FLASH_ACR_LATENCY_2;

    // Bus clock
    RCC->CFGR &= ~(RCC_CFGR_HPRE);
    RCC->CFGR |= RCC_CFGR_HPRE_DIV1;

    RCC->CFGR &= ~(RCC_CFGR_PPRE1);
    RCC->CFGR |= RCC_CFGR_PPRE1_DIV2;

    RCC->CFGR &= ~(RCC_CFGR_PPRE2);
    RCC->CFGR |= RCC_CFGR_PPRE2_DIV1;

    // Configure PLL
    RCC->CFGR &= ~(RCC_CFGR_PLLMULL | RCC_CFGR_PLLXTPRE | RCC_CFGR_PLLSRC);
    RCC->CFGR |= (RCC_CFGR_PLLMULL9 | RCC_CFGR_PLLSRC | RCC_CFGR_PLLXTPRE_HSE);

    // Enable PLL
    RCC->CR |= RCC_CR_PLLON;
    while ((RCC->CR & RCC_CR_PLLRDY) == 0);

    // Select PLL as SYSCLK
    RCC->CFGR &= ~(RCC_CFGR_SW);
    RCC->CFGR |= RCC_CFGR_SW_PLL;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);
}
