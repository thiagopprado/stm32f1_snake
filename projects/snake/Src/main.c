#include <stdint.h>

#include "gpio.h"
#include "spi.h"
#include "nokia5110.h"
#include "snake.h"

void delay_ms(uint32_t millis) {
    volatile uint32_t a = 0;

    for (; millis > 0; millis--) {
        for (a = 0; a < 800; a++);
    }
}

int main(void) {
    uint8_t i = 0;

    spi_setup(SPI_BUS_1);
    nokia5110_setup();
    nokia5110_clear_buffer();
    nokia5110_update_screen();

    snake_init();
    nokia5110_update_screen();

    for(;;) {
        i++;

        snake_kbd_debounce();
        if (i == 100) {
            // Update game each ~100ms
            snake_update();
            nokia5110_update_screen();
            i = 0;
        }

        delay_ms(1);
    }
}
