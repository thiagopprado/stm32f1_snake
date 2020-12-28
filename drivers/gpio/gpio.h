#ifndef GPIO_H
#define GPIO_H

#include <stdint.h>
#include "stm32f1xx.h"

#define GPIO_MAX_PIN_NR     16

typedef enum {
    GPIO_PORTA = 0,
    GPIO_PORTB,
    GPIO_PORTC,
    GPIO_PORTD,
    GPIO_PORTE,
} gpio_port_t;

typedef enum {
    GPIO_ERR_NO_ERROR = 0,
    GPIO_ERR_INVALID,
} gpio_err_t;

typedef enum {
    GPIO_MODE_INPUT = 0,    /**< Input Mode (Reset State) */
    GPIO_MODE_OUTPUT_10,    /**< Output Mode (Max speed 10 Mhz) */
    GPIO_MODE_OUTPUT_2,     /**< Output Mode (Max speed 2 Mhz) */
    GPIO_MODE_OUTPUT_50,    /**< Output Mode (Max speed 50 Mhz) */
} gpio_mode_t;

typedef enum {
    // Input mode cfg
    GPIO_CFG_IN_ANALOG = 0,
    GPIO_CFG_IN_FLOAT,
    GPIO_CFG_IN_PULL,
    // Output mode cfg
    GPIO_CFG_OUT_PUSH_PULL = 0,
    GPIO_CFG_OUT_OPEN_DRAIN,
    GPIO_CFG_OUT_AF_PUSH_PULL,
    GPIO_CFG_OUT_AF_OPEN_DRAIN,
} gpio_cfg_t;

typedef enum {
    GPIO_STATE_LOW = 0,
    GPIO_STATE_HIGH,
} gpio_state_t;

gpio_err_t gpio_setup(gpio_port_t port, uint8_t pin, gpio_mode_t mode, gpio_cfg_t cfg);
gpio_err_t gpio_write(gpio_port_t port, uint8_t pin, gpio_state_t state);
gpio_state_t gpio_read(gpio_port_t port, uint8_t pin);
// Atomic functions
void gpio_set(GPIO_TypeDef* gpio, uint8_t pin);
void gpio_clr(GPIO_TypeDef* gpio, uint8_t pin);

#endif /* GPIO_H */