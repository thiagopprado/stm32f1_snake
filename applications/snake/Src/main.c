#include <stdint.h>
#include <stdbool.h>

#include "gpio.h"
#include "rcc.h"
#include "spi.h"
#include "timer.h"
#include "nokia5110.h"
#include "snake.h"

static volatile bool timer_wait_flag = true;

static void timer_wait_callback(void) {
    timer_wait_flag = false;
}

int main(void) {
    uint8_t i = 0;

    rcc_clock_init();

    spi_setup(SPI_BUS_1);
    nokia5110_setup();
    nokia5110_clear_buffer();
    nokia5110_update_screen();

    // Timer period = 1ms
    timer_setup(10);
    timer_attach_callback(timer_wait_callback);

    snake_init();
    nokia5110_update_screen();

    for(;;) {
        // Wait for timer ISR
        while (timer_wait_flag == true);
        timer_wait_flag = true;

        snake_kbd_debounce();

        i++;
        if (i >= 100) {
            i = 0;

            // Update game each ~100ms
            snake_update();
            nokia5110_update_screen();
        }
    }
}
