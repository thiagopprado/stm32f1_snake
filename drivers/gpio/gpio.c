#include "gpio.h"

#include <stddef.h>

gpio_err_t gpio_setup(gpio_port_t port, uint8_t pin, gpio_mode_t mode, gpio_cfg_t cfg) {
    GPIO_TypeDef* gpio_ptr = NULL;

    switch (port) {
        case GPIO_PORTA:
            gpio_ptr = GPIOA;
            RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
        break;
        case GPIO_PORTB:
            gpio_ptr = GPIOB;
            RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
        break;
        case GPIO_PORTC:
            gpio_ptr = GPIOC;
            RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;
        break;
        case GPIO_PORTD:
            gpio_ptr = GPIOD;
            RCC->APB2ENR |= RCC_APB2ENR_IOPDEN;
        break;
        case GPIO_PORTE:
            gpio_ptr = GPIOE;
            RCC->APB2ENR |= RCC_APB2ENR_IOPEEN;
        break;
        default:
            return GPIO_ERR_INVALID;
    }

    // Calculates MODE and CFG registers position for the correspondent pin
    if (pin < 8) {
        uint8_t reg_pos = pin * 4;
        gpio_ptr->CRL = (gpio_ptr->CRL & ~(0x000F << reg_pos)) | (((cfg << 2) + mode) << reg_pos);
    } else if (pin < GPIO_MAX_PIN_NR) {
        uint8_t reg_pos = (pin - 8) * 4;
        gpio_ptr->CRH = (gpio_ptr->CRH & ~(0x000F << reg_pos)) | (((cfg << 2) + mode) << reg_pos);
    } else {
        return GPIO_ERR_INVALID;
    }

    return GPIO_ERR_NO_ERROR;
}

gpio_state_t gpio_read(gpio_port_t port, uint8_t pin) {
    GPIO_TypeDef* gpio_ptr = NULL;

    switch (port) {
        case GPIO_PORTA:
            gpio_ptr = GPIOA;
        break;
        case GPIO_PORTB:
            gpio_ptr = GPIOB;
        break;
        case GPIO_PORTC:
            gpio_ptr = GPIOC;
        break;
        case GPIO_PORTD:
            gpio_ptr = GPIOD;
        break;
        case GPIO_PORTE:
            gpio_ptr = GPIOE;
        break;
        default:
            return GPIO_STATE_LOW;
    }

    if (pin >= GPIO_MAX_PIN_NR) {
        return GPIO_STATE_LOW;
    }

    return (gpio_ptr->IDR >> pin) & 0x0001;
}

gpio_err_t gpio_write(gpio_port_t port, uint8_t pin, gpio_state_t state) {
    GPIO_TypeDef* gpio_ptr = NULL;

    switch (port) {
        case GPIO_PORTA:
            gpio_ptr = GPIOA;
        break;
        case GPIO_PORTB:
            gpio_ptr = GPIOB;
        break;
        case GPIO_PORTC:
            gpio_ptr = GPIOC;
        break;
        case GPIO_PORTD:
            gpio_ptr = GPIOD;
        break;
        case GPIO_PORTE:
            gpio_ptr = GPIOE;
        break;
        default:
            return GPIO_ERR_INVALID;
    }

    if (pin >= GPIO_MAX_PIN_NR) {
        return GPIO_ERR_INVALID;
    }

    if (state == GPIO_STATE_LOW) {
        gpio_ptr->ODR &= ~(1 << pin);
    } else {
        gpio_ptr->ODR |= (1 << pin);
    }

    return GPIO_ERR_NO_ERROR;
}

inline void gpio_set(GPIO_TypeDef* gpio, uint8_t pin) {
    gpio->BSRR = (1 << pin);
}

inline void gpio_clr(GPIO_TypeDef* gpio, uint8_t pin) {
    gpio->BRR = (1 << pin);
}
