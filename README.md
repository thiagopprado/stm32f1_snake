
# STM32F1 - Blue Pill

This repository contains drivers and projects using the blue pill board, which contains the STM32F1 microcontroller.

## Drivers
Currently were implemented the following drivers:
 - [GPIO](https://github.com/thiagopprado/STM32F1/tree/master/drivers/gpio)
 - [SPI](https://github.com/thiagopprado/STM32F1/tree/master/drivers/spi)
 - [Nokia 5110 display](https://github.com/thiagopprado/STM32F1/tree/master/drivers/nokia5110)

## Projects
### Snake
The first project implemented is a version of the classic **[Snake game](https://github.com/thiagopprado/STM32F1/tree/master/snake)**, which became famous on the first Nokia phones.
It has 4 mecanic keys connected to the MCU GPIO, for character control and a Nokia 5110 LED display.
Here is a sample of the code running:

![](snake_example.gif)
