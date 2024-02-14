/**
 * @file
 * @brief Snake game main file.
 */
/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>

#include "gpio.h"
#include "spi.h"
#include "nokia5110.h"
#include "snake.h"

#include "stm32f1xx_hal.h"

/* Private types -------------------------------------------------------------*/

/* Private defines -----------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
static void clock_config(void);

/* Private function implementation--------------------------------------------*/

/**
 * @brief MCU clock configuration.
 */
static void clock_config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
    RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
    HAL_RCC_OscConfig(&RCC_OscInitStruct);

    /** Initializes the CPU, AHB and APB buses clocks
     */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);
}

/* Public functions ----------------------------------------------------------*/
/**
 * @brief Main function.
 */
int main(void) {
    uint32_t game_update_timeshot = 0;

    HAL_Init();
    clock_config();

    spi_setup(SPI_BUS_1);
    nokia5110_setup();
    nokia5110_clear_buffer();
    nokia5110_update_screen();

    snake_init();
    nokia5110_update_screen();

    for(;;) {
        snake_kbd_debounce();

        if (HAL_GetTick() - game_update_timeshot >= 100) {
            game_update_timeshot = HAL_GetTick();
            snake_update();
            nokia5110_update_screen();
        }
    }
}
