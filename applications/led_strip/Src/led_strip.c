/** Includes ------------------------------------------------------ */
#include "led_strip.h"

#include <stdbool.h>
#include <string.h>

#include "led_ws2812.h"

#include "gpio.h"
#include "infrared.h"

/** Definitions --------------------------------------------------- */
#define LED_FADE_BRIGHT_STEP_SLOW   1
#define LED_FADE_BRIGHT_STEP_FAST   5

#define LED_SNAKE_SIZE          10
#define LED_SNAKE_BRIGHT_MAX    LED_WS2812_COLOR_MAX
#define LED_SNAKE_BRIGHT_STEP   (LED_WS2812_COLOR_MAX / (LED_SNAKE_SIZE - 1))
#define LED_SNAKE_DELAY         4

#define LED_CONTROL_DELAY_FAST      20
#define LED_CONTROL_DELAY_SLOW      50
#define LED_CONTROL_BRIGHT_STEP     10

#define KEY_HOLD_COUNTER            200

/** Types --------------------------------------------------------- */
typedef enum {
    LED_COLOR_MODE_STATIC = 0,
    LED_COLOR_MODE_PULSE,
} led_color_mode_t;

typedef enum {
    LED_PULSE_UP = 0,
    LED_PULSE_DOWN,
} led_pulse_dir_t;

typedef enum {
    LED_XMAS_FADE = 0,
    LED_XMAS_SNAKE,
} led_xmas_t;

typedef enum {
    LED_FADE_1 = 0,
    LED_FADE_2,
    LED_FADE_3,
    LED_FADE_4,
    LED_FADE_5,
} led_fade_state_t;

typedef enum {
    LED_FADE_UP = 0,
    LED_FADE_DOWN,
} led_fade_dir_t;

typedef enum {
    LED_FADE_SLOW = 0,
    LED_FADE_FAST,
} led_fade_speed_t;

typedef enum {
    LED_SNAKE_RED = 0,
    LED_SNAKE_YELLOW,
    LED_SNAKE_GREEN,
} led_snake_state_t;

typedef enum {
    LED_EFFECT_RED = 0,
    LED_EFFECT_GREEN,
    LED_EFFECT_BLUE,
    LED_EFFECT_YELLOW,
    LED_EFFECT_MAGENTA,
    LED_EFFECT_CYAN,
    LED_EFFECT_PURPLE,
    LED_EFFECT_WHITE,
    LED_EFFECT_RED_BLUE,
    LED_EFFECT_XMAS,
} led_effect_t;

/** Variables ----------------------------------------------------- */
static uint32_t led_color[LED_WS2812_NR] = { 0 };
static int32_t led_bright = LED_WS2812_COLOR_MAX;

static led_color_mode_t led_color_mode = LED_COLOR_MODE_STATIC;
static led_pulse_dir_t led_pulse_dir = LED_PULSE_UP;
static int32_t led_pulse_bright = 0;

static led_effect_t led_effect = LED_EFFECT_RED;

static led_xmas_t led_xmas_state = LED_XMAS_FADE;

static led_fade_state_t led_fade_state = LED_FADE_1;
static led_fade_dir_t led_fade_dir = LED_FADE_UP;
static led_fade_speed_t led_fade_speed = LED_FADE_SLOW;
static int32_t led_fade_bright = 0;

static led_snake_state_t led_snake_state = LED_SNAKE_RED;
static uint32_t led_snake_head = 0;

/** Prototypes ---------------------------------------------------- */

static void led_effect_color(led_effect_t effect);
static void led_effect_xmas(void);
static bool led_xmas_fade(void);
static bool led_xmas_snake(void);

/** Internal functions -------------------------------------------- */
/**
 * @brief Updates LEDs with a fixed color.
 * 
 * @param effect    Effect to update.
 */
static void led_effect_color(led_effect_t effect) {
    static uint32_t pulse_delay = 0;
    int32_t color_bright = 0;
    uint32_t i = 0;

    if (pulse_delay > 0) {
        pulse_delay--;
    }

    if (led_color_mode == LED_COLOR_MODE_STATIC) {
        color_bright = led_bright;
    } else if (led_color_mode == LED_COLOR_MODE_PULSE) {
        color_bright = led_pulse_bright;

        if (pulse_delay == 0) {
            pulse_delay = 3;
            if (led_pulse_dir == LED_PULSE_UP) {
                led_pulse_bright++;
                if (led_pulse_bright > LED_WS2812_COLOR_MAX) {
                    led_pulse_bright = LED_WS2812_COLOR_MAX;
                    led_pulse_dir = LED_PULSE_DOWN;
                }

            } else {
                led_pulse_bright--;
                if (led_pulse_bright < 0) {
                    led_pulse_bright = 0;
                    led_pulse_dir = LED_PULSE_UP;
                }
            }
        }
    }

    for (i = 0; i < LED_WS2812_NR; i++) {
        switch (led_effect) {
            case LED_EFFECT_RED: {
                led_color[i] = LED_WS2812_GET_R(color_bright);
                break;
            }
            case LED_EFFECT_GREEN: {
                led_color[i] = LED_WS2812_GET_G(color_bright);
                break;
            }
            case LED_EFFECT_BLUE: {
                led_color[i] = LED_WS2812_GET_B(color_bright);
                break;
            }
            case LED_EFFECT_YELLOW: {
                led_color[i] = LED_WS2812_GET_R(color_bright) | LED_WS2812_GET_G(color_bright);
                break;
            }
            case LED_EFFECT_MAGENTA: {
                led_color[i] = LED_WS2812_GET_R(color_bright) | LED_WS2812_GET_B(color_bright);
                break;
            }
            case LED_EFFECT_CYAN: {
                led_color[i] = LED_WS2812_GET_G(color_bright) | LED_WS2812_GET_B(color_bright);
                break;
            }
            case LED_EFFECT_PURPLE: {
                led_color[i] = LED_WS2812_GET_R(color_bright / 2) | LED_WS2812_GET_B(color_bright);
                break;
            }
            case LED_EFFECT_WHITE: {
                led_color[i] = LED_WS2812_GET_R(color_bright) | LED_WS2812_GET_G(color_bright) | LED_WS2812_GET_B(color_bright);
                break;
            }
            case LED_EFFECT_RED_BLUE: {
                if (i >= (LED_WS2812_NR / 3) && i < ((LED_WS2812_NR * 2) / 3)) {
                    led_color[i] = LED_WS2812_GET_G(color_bright / 10) | LED_WS2812_GET_B(color_bright);
                } else {
                    led_color[i] = LED_WS2812_GET_R(color_bright) | LED_WS2812_GET_B(color_bright / 20);
                }
                break;
            }
            default: {
                break;
            }
        }
    }
}

/**
 * @brief Christmas light effect.
 */
static void led_effect_xmas(void) {
    switch (led_xmas_state) {
        case LED_XMAS_FADE: {
            if (led_xmas_fade() == true) {
                led_xmas_state = LED_XMAS_SNAKE;
            }
            break;
        }
        case LED_XMAS_SNAKE: {
            if (led_xmas_snake() == true) {
                led_xmas_state = LED_XMAS_FADE;
            }
            break;
        }
        default: {
            break;
        }
    }
}

/**
 * @brief Updates LED strip with fade effect.
 * 
 * @return true when the effect is finished
 */
static bool led_xmas_fade(void) {
    bool effect_finish = false;
    uint32_t bright_step = 0;
    uint32_t i = 0;

    // Prints the effect
    switch (led_fade_state) {
        case LED_FADE_1:
        case LED_FADE_5: {
            for (i = 0; i < LED_WS2812_NR; i += 4) {
                led_color[i] = LED_WS2812_GET_R(led_fade_bright) | LED_WS2812_GET_G(led_fade_bright);
            }
            for (i = 1; i < LED_WS2812_NR; i += 4) {
                led_color[i] = LED_WS2812_GET_R(LED_WS2812_COLOR_MAX - led_fade_bright);
            }
            for (i = 2; i < LED_WS2812_NR; i += 4) {
                led_color[i] = LED_WS2812_GET_G(led_fade_bright);
            }
            for (i = 3; i < LED_WS2812_NR; i += 4) {
                led_color[i] = LED_WS2812_GET_R(LED_WS2812_COLOR_MAX - led_fade_bright);
            }

            break;
        }
        case LED_FADE_2:
        case LED_FADE_4: {
            for (i = 0; i < LED_WS2812_NR; i += 4) {
                led_color[i] = LED_WS2812_GET_G(led_fade_bright);
            }
            for (i = 1; i < LED_WS2812_NR; i += 4) {
                led_color[i] = LED_WS2812_GET_R(LED_WS2812_COLOR_MAX - led_fade_bright) | LED_WS2812_GET_G(LED_WS2812_COLOR_MAX - led_fade_bright);
            }
            for (i = 2; i < LED_WS2812_NR; i += 4) {
                led_color[i] = LED_WS2812_GET_R(led_fade_bright);
            }
            for (i = 3; i < LED_WS2812_NR; i += 4) {
                led_color[i] = LED_WS2812_GET_R(LED_WS2812_COLOR_MAX - led_fade_bright) | LED_WS2812_GET_G(LED_WS2812_COLOR_MAX - led_fade_bright);
            }

            break;
        }
        case LED_FADE_3: {
            for (i = 0; i < LED_WS2812_NR; i += 2) {
                led_color[i] = LED_WS2812_GET_R(led_fade_bright) | LED_WS2812_GET_G(led_fade_bright);
            }

            for (i = 1; i < LED_WS2812_NR; i += 2) {
                led_color[i] = LED_WS2812_GET_R(LED_WS2812_COLOR_MAX - led_fade_bright) | LED_WS2812_GET_G(LED_WS2812_COLOR_MAX - led_fade_bright);
            }

            break;
        }
        default: {
            break;
        }
    }

    if (led_fade_speed == LED_FADE_SLOW) {
        bright_step = LED_FADE_BRIGHT_STEP_SLOW;
    } else {
        bright_step = LED_FADE_BRIGHT_STEP_FAST;
    }

    // Updates the bright
    if (led_fade_dir == LED_FADE_UP) {
        led_fade_bright += bright_step;
        if (led_fade_bright > LED_WS2812_COLOR_MAX) {
            led_fade_bright = LED_WS2812_COLOR_MAX;
            // Updates the fade direction
            led_fade_dir = LED_FADE_DOWN;
        }
    } else {
        led_fade_bright -= bright_step;
        if (led_fade_bright < 0) {
            led_fade_bright = 0;
            led_fade_dir = LED_FADE_UP;

            // Updates the fade pattern
            led_fade_state++;
            if (led_fade_state > LED_FADE_5) {
                led_fade_state = LED_FADE_1;

                // Updates the fade speed
                if (led_fade_speed == LED_FADE_SLOW) {
                    led_fade_speed = LED_FADE_FAST;
                } else {
                    led_fade_speed = LED_FADE_SLOW;
                    effect_finish = true;
                }
            }
        }
    }

    return effect_finish;
}

/**
 * @brief Updates LED strip with snake effect.
 * 
 * @return true when the effect is finished
 */
static bool led_xmas_snake(void) {
    static uint32_t time_counter = 0;
    bool effect_finish = false;
    int32_t i = 0;

    // Prints the snake from the head
    for (i = 0; i < LED_SNAKE_SIZE; i++) {
        int32_t led_pos = led_snake_head - i;

        if (led_pos >= 0 && led_pos < LED_WS2812_NR) {
            uint32_t led_bright = LED_SNAKE_BRIGHT_MAX - i * LED_SNAKE_BRIGHT_STEP;

            if (led_snake_state == LED_SNAKE_RED) {
                led_color[led_pos] = LED_WS2812_GET_R(led_bright);

            } else if (led_snake_state == LED_SNAKE_GREEN) {
                led_color[led_pos] = LED_WS2812_GET_G(led_bright);

            } else if (led_snake_state == LED_SNAKE_YELLOW) {
                led_color[led_pos] = LED_WS2812_GET_R(led_bright) | LED_WS2812_GET_G(led_bright);
            }
        }
    }

    time_counter++;
    if (time_counter > LED_SNAKE_DELAY) {
        time_counter = 0;

        // Updates snake head
        led_snake_head++;
        if (led_snake_head >= LED_WS2812_NR + LED_SNAKE_SIZE) {
            led_snake_head = 0;

            // Updates snake color
            led_snake_state++;
            if (led_snake_state > LED_SNAKE_GREEN) {
                led_snake_state = LED_SNAKE_RED;
                effect_finish = true;
            }
        }
    }

    return effect_finish;
}

/** Public functions ---------------------------------------------- */
/**
 * @brief Sets up LED strip application.
 */
void led_setup(void)
{
    infrared_setup();
    led_ws2812_setup();

    gpio_setup(GPIO_PORTB, 7, GPIO_MODE_INPUT, GPIO_CFG_IN_PULL);
    gpio_write(GPIO_PORTB, 7, GPIO_STATE_HIGH); // Pull up
}

/**
 * @brief Updates the LED strip.
 */
void led_update(void) {
    static uint32_t ir_read_cooldown = 0;
    ir_key_id_t key_pressed = infrared_decode();

    static bool last_key_state = true;
    static uint32_t key_hold_cnt = 0;
    bool key_state = gpio_read(GPIO_PORTB, 7);

    memset(led_color, 0, sizeof(led_color));

    if (ir_read_cooldown > 0) {
        ir_read_cooldown--;
    }

    if (last_key_state == false && key_state == true && key_hold_cnt < KEY_HOLD_COUNTER) {
        key_pressed = INFRARED_KEY_RIGHT;
    }

    if (key_state == false) {
        key_hold_cnt++;
        if (key_hold_cnt == KEY_HOLD_COUNTER) {
            key_pressed = INFRARED_KEY_ENTER;
        }
    } else {
        key_hold_cnt = 0;
    }

    last_key_state = key_state;

    if (key_pressed == INFRARED_KEY_RIGHT && ir_read_cooldown == 0) {
        if (led_effect == LED_EFFECT_XMAS) {
            led_effect = LED_EFFECT_RED;
        } else {
            led_effect++;
        }

        ir_read_cooldown = LED_CONTROL_DELAY_FAST;

    } else if (key_pressed == INFRARED_KEY_LEFT && ir_read_cooldown == 0) {
        if (led_effect == LED_EFFECT_RED) {
            led_effect = LED_EFFECT_XMAS;
        } else {
            led_effect--;
        }

        ir_read_cooldown = LED_CONTROL_DELAY_FAST;

    } else if (key_pressed == INFRARED_KEY_ENTER && ir_read_cooldown == 0) {
        if (led_color_mode == LED_COLOR_MODE_STATIC) {
            led_color_mode = LED_COLOR_MODE_PULSE;
            led_pulse_bright = led_bright;

        } else if (led_color_mode == LED_COLOR_MODE_PULSE) {
            led_color_mode = LED_COLOR_MODE_STATIC;
        }

        ir_read_cooldown = LED_CONTROL_DELAY_FAST;

    } else if (key_pressed == INFRARED_KEY_UP) {
        led_bright += LED_CONTROL_BRIGHT_STEP;
        if (led_bright > LED_WS2812_COLOR_MAX) {
            led_bright = LED_WS2812_COLOR_MAX;
        }

    } else if (key_pressed == INFRARED_KEY_DOWN) {
        led_bright -= LED_CONTROL_BRIGHT_STEP;
        if (led_bright < 0) {
            led_bright = 0;
        }
    }

    switch (led_effect) {
        case LED_EFFECT_RED:
        case LED_EFFECT_GREEN:
        case LED_EFFECT_BLUE:
        case LED_EFFECT_YELLOW:
        case LED_EFFECT_MAGENTA:
        case LED_EFFECT_CYAN:
        case LED_EFFECT_PURPLE:
        case LED_EFFECT_WHITE:
        case LED_EFFECT_RED_BLUE: {
            led_effect_color(led_effect);
            break;
        }
        case LED_EFFECT_XMAS: {
            led_effect_xmas();
            break;
        }
    }

    led_ws2812_write(led_color, LED_WS2812_NR);
}
