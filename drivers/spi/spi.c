#include "spi.h"

#include <stddef.h>
#include "stm32f1xx.h"

#include "gpio.h"

void spi_setup(spi_bus_t spi) {
	RCC->APB2ENR |= RCC_APB2ENR_AFIOEN; // Enable Alternate Function

    if (spi == SPI_BUS_1) {
        RCC->APB2ENR |= RCC_APB2ENR_SPI1EN; // Enable SPI1

        gpio_setup(GPIO_PORTA, 5, GPIO_MODE_OUTPUT_50, GPIO_CFG_OUT_AF_PUSH_PULL);
        gpio_setup(GPIO_PORTA, 6, GPIO_MODE_INPUT, GPIO_CFG_IN_FLOAT);
        gpio_setup(GPIO_PORTA, 7, GPIO_MODE_OUTPUT_50, GPIO_CFG_OUT_AF_PUSH_PULL);

        /**
         *  Prescaler f/8 --> 1 Mhz
         *  CPO = 0, CPHA = 0 --> Clock IDLE low, rising edge
         *  8 bits data frame
         *  MSB first
         *  Master mode
         *  Software slave management --> NSS = 1
         */
        SPI1->CR1 = (2 << SPI_CR1_BR_Pos) | SPI_CR1_MSTR | SPI_CR1_SSI | SPI_CR1_SSM;
        SPI1->CR1 |= SPI_CR1_SPE; // SPI Enable
    } else {
        // Not implemented yet!
    }
}

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
