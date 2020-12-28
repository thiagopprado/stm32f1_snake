/**
 * Author: thiagopereiraprado@gmail.com
 * 
 * @file
 * @defgroup gpio STM32F1 GPIO driver
 * @brief STM32F1 GPIO driver
 * 
 */
#ifndef GPIO_H
#define GPIO_H

#include <stdint.h>
#include "stm32f1xx.h"

#define GPIO_MAX_PIN_NR     16

/**
 * @ingroup gpio
 * @brief GPIO error codes.
 */
typedef enum {
    GPIO_ERR_NO_ERROR = 0,  /**< No error ocurred. */
    GPIO_ERR_INVALID,       /**< An invalid parameter was informed. */
} gpio_err_t;

/**
 * @ingroup gpio
 * @brief GPIO available ports enumeration.
 */
typedef enum {
    GPIO_PORTA = 0,     /**< PORTA. */
    GPIO_PORTB,         /**< PORTB. */
    GPIO_PORTC,         /**< PORTC. */
    GPIO_PORTD,         /**< PORTD. */
    GPIO_PORTE,         /**< PORTE. */
} gpio_port_t;

/**
 * @ingroup gpio
 * @brief GPIO pin modes.
 */
typedef enum {
    GPIO_MODE_INPUT = 0,    /**< Input Mode (Reset State). */
    GPIO_MODE_OUTPUT_10,    /**< Output Mode (Max speed 10 Mhz). */
    GPIO_MODE_OUTPUT_2,     /**< Output Mode (Max speed 2 Mhz). */
    GPIO_MODE_OUTPUT_50,    /**< Output Mode (Max speed 50 Mhz). */
} gpio_mode_t;

/**
 * @ingroup gpio
 * @brief GPIO pin configurations.
 *
 * @note When configuring an input pin with an internal pull resistor
 * the GPIOx->ODR register must be used to configure if it will be a
 * pull-up or pull-down resistor.
 * It can be done using the @ref gpio_write function. After configuring
 * the input pull mode on a pin, writing GPIO_STATE_HIGH to it will 
 * configure a pull-up resistor, GPIO_STATE_LOW, pull-down.
 */
typedef enum {
    // Input mode cfg
    GPIO_CFG_IN_ANALOG = 0,     /**< Analog input. */
    GPIO_CFG_IN_FLOAT,          /**< Floating input. */
    GPIO_CFG_IN_PULL,           /**< Internal pull-up/down input. */
    // Output mode cfg
    GPIO_CFG_OUT_PUSH_PULL = 0, /**< Push pull output. */
    GPIO_CFG_OUT_OPEN_DRAIN,    /**< Open drain output. */
    GPIO_CFG_OUT_AF_PUSH_PULL,  /**< Push pull alternate function output. */
    GPIO_CFG_OUT_AF_OPEN_DRAIN, /**< Open drain alternate function output. */
} gpio_cfg_t;

/**
 * @ingroup gpio
 * @brief GPIO logic levels.
 */
typedef enum {
    GPIO_STATE_LOW = 0, /**< Low logic level. */
    GPIO_STATE_HIGH,    /**< High logic level. */
} gpio_state_t;

gpio_err_t gpio_setup(gpio_port_t port, uint8_t pin, gpio_mode_t mode, gpio_cfg_t cfg);
gpio_err_t gpio_write(gpio_port_t port, uint8_t pin, gpio_state_t state);
gpio_state_t gpio_read(gpio_port_t port, uint8_t pin);
// Atomic functions
void gpio_set(GPIO_TypeDef* gpio, uint8_t pin);
void gpio_clr(GPIO_TypeDef* gpio, uint8_t pin);

#endif /* GPIO_H */