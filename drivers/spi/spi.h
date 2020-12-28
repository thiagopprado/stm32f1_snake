/**
 * Author: thiagopereiraprado@gmail.com
 * 
 * @file
 * @defgroup spi STM32F1 SPI driver
 * @brief STM32F1 SPI driver
 * 
 */
#ifndef SPI_H
#define SPI_H

#include <stdint.h>

/**
 * @ingroup spi
 * @brief SPI available buses.
 */
typedef enum {
    SPI_BUS_1 = 0,  /**< SPI1. */
    SPI_BUS_2,      /**< SPI2. */
    SPI_BUS_3,      /**< SPI3. */
} spi_bus_t;

void spi_setup(spi_bus_t spi);
void spi_trx(spi_bus_t spi, uint8_t* buffer, uint16_t trx_size);

#endif /* SPI_H */
