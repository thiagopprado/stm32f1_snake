/**
 * Author: thiagopereiraprado@gmail.com
 * 
 * @file
 * @ingroup spi
 * @brief STM32F1 SPI driver
 * 
 */
#include "spi.h"

#include <stddef.h>
#include "stm32f1xx.h"

#include "gpio.h"

/**
 * @ingroup spi
 * @brief Sets up the SPI bus.
 *
 * @param spi   SPI bus to set up.
 *
 * @note The chip select pin must be set up and handled outside
 * the driver.
 */
void spi_setup(spi_bus_t spi) {
    // Enable Alternate Function
    RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;

    if (spi == SPI_BUS_1) {
        // Enable SPI1
        RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;

        // PORTA5 = SCLK1
        gpio_setup(GPIO_PORTA, 5, GPIO_MODE_OUTPUT_50, GPIO_CFG_OUT_AF_PUSH_PULL);
        // PORTA6 = MISO1
        gpio_setup(GPIO_PORTA, 6, GPIO_MODE_INPUT, GPIO_CFG_IN_FLOAT);
        // PORTA7 = MOSI1
        gpio_setup(GPIO_PORTA, 7, GPIO_MODE_OUTPUT_50, GPIO_CFG_OUT_AF_PUSH_PULL);

        /**
         *  Prescaler f/64 --> 1.125 Mhz
         *  CPO = 0, CPHA = 0 --> Clock IDLE low, rising edge
         *  8 bits data frame
         *  MSB first
         *  Master mode
         *  Software slave management --> NSS = 1
         */
        SPI1->CR1 = SPI_CR1_BR_2 | SPI_CR1_BR_0 | SPI_CR1_MSTR | SPI_CR1_SSI | SPI_CR1_SSM;
        // SPI Enable
        SPI1->CR1 |= SPI_CR1_SPE;
    } else {
        // Not implemented yet!
    }
}

/**
 * @ingroup spi
 * @brief Starts a SPI transmission.
 *
 * @param spi       SPI bus.
 * @param buffer    Pointer to the buffer with the message to send (will be overwriten with the read message).
 * @param trx_size  Transmission's size.
 */
void spi_trx(spi_bus_t spi, uint8_t* buffer, uint16_t trx_size) {
    uint16_t write_idx = 0, read_idx = 0;

    if (spi == SPI_BUS_1) {
        while (write_idx < trx_size || read_idx < trx_size) {
            // Transmit next byte
            if (((SPI1->SR & SPI_SR_TXE) != 0) && (write_idx < trx_size)) {
                SPI1->DR = buffer[write_idx];
                write_idx++;
            }

            // Waits busy flag
            while ((SPI1->SR & SPI_SR_BSY) != 0);

            // Read byte
            if (((SPI1->SR & SPI_SR_RXNE) != 0) && (read_idx < trx_size)) {
                buffer[read_idx] = SPI1->DR;
                read_idx++;
            }

            asm("NOP");
        }
    } else {
        // Not implemented yet!
    }
}
