#include <stdint.h>
#include "stm32f1xx.h"

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"

#include "gpio.h"
#include "rcc.h"

#define seconds(val)        (val * configTICK_RATE_HZ)
#define milliseconds(val)   (val * configTICK_RATE_HZ / 1000)

void task_blink(void *param) {
    gpio_state_t state = GPIO_STATE_HIGH;
    TickType_t last_wake_time = 0;

    gpio_setup(GPIO_PORTC, 13, GPIO_MODE_OUTPUT_50, GPIO_CFG_OUT_PUSH_PULL);
    last_wake_time = xTaskGetTickCount();

    while (1) {
        xTaskDelayUntil(&last_wake_time, milliseconds(1000));

        gpio_write(GPIO_PORTC, 13, state);
        state = !state;
    }
}

int main(void) {
	rcc_clock_init();

    xTaskCreate(task_blink, "Task blink", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, NULL);
    vTaskStartScheduler();
}
