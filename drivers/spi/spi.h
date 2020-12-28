#ifndef SPI_H
#define SPI_H

#include <stdint.h>

typedef enum {
    SPI_BUS_1 = 0,
    SPI_BUS_2,
    SPI_BUS_3,
} spi_bus_t;

void spi_setup(spi_bus_t spi);
void spi_trx(spi_bus_t spi, uint8_t* buffer, uint16_t trx_size);

#endif /* SPI_H */
