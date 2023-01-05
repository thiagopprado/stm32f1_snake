#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "stm32f1xx.h"
#include "core_cm3.h"

#include "rcc.h"
#include "led_ws2812.h"
#include "timer.h"

#include "buzzer.h"
#include "infrared.h"

/** Definitions --------------------------------------------------- */
#define LED_FADE_BRIGHT_STEP_SLOW   1
#define LED_FADE_BRIGHT_STEP_FAST   5

/**
 * @brief Times.
 * 
 * Timer counter is incremented every 100us.
 * 
 * @{
 */
#define LED_UPDATE_TIME         50
#define BUZZER_NOTE_TIME        1000
/** @} */

/** Types --------------------------------------------------------- */
typedef enum {
    LED_EFFECT_FADE = 0,
} led_effect_t;

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

/** Variables ----------------------------------------------------- */
static volatile uint32_t timer_counter = 0;

static buzzer_note_t sheet_music[] = {
    BUZZER_NOTE_E4, BUZZER_NOTE_ST, BUZZER_NOTE_E4, BUZZER_NOTE_ST,
    BUZZER_NOTE_E4, BUZZER_NOTE_E4, BUZZER_NOTE_E4, BUZZER_NOTE_ST,
    BUZZER_NOTE_E4, BUZZER_NOTE_ST, BUZZER_NOTE_E4, BUZZER_NOTE_ST,
    BUZZER_NOTE_E4, BUZZER_NOTE_E4, BUZZER_NOTE_E4, BUZZER_NOTE_ST,
    BUZZER_NOTE_E4, BUZZER_NOTE_ST, BUZZER_NOTE_G4, BUZZER_NOTE_ST,
    BUZZER_NOTE_C4, BUZZER_NOTE_ST, BUZZER_NOTE_D4, BUZZER_NOTE_ST,
    BUZZER_NOTE_E4, BUZZER_NOTE_E4, BUZZER_NOTE_E4, BUZZER_NOTE_E4,
    BUZZER_NOTE_E4, BUZZER_NOTE_E4, BUZZER_NOTE_E4, BUZZER_NOTE_E4,
    BUZZER_NOTE_F4, BUZZER_NOTE_ST, BUZZER_NOTE_F4, BUZZER_NOTE_ST,
    BUZZER_NOTE_F4, BUZZER_NOTE_F4, BUZZER_NOTE_F4, BUZZER_NOTE_ST,
    BUZZER_NOTE_F4, BUZZER_NOTE_ST, BUZZER_NOTE_E4, BUZZER_NOTE_ST,
    BUZZER_NOTE_E4, BUZZER_NOTE_ST, BUZZER_NOTE_E4, BUZZER_NOTE_ST,
    BUZZER_NOTE_G4, BUZZER_NOTE_ST, BUZZER_NOTE_G4, BUZZER_NOTE_ST,
    BUZZER_NOTE_F4, BUZZER_NOTE_ST, BUZZER_NOTE_D4, BUZZER_NOTE_ST,
    BUZZER_NOTE_C4, BUZZER_NOTE_C4, BUZZER_NOTE_C4, BUZZER_NOTE_C4,
    BUZZER_NOTE_C4, BUZZER_NOTE_C4, BUZZER_NOTE_C4, BUZZER_NOTE_C4,
    BUZZER_NOTE_ST, BUZZER_NOTE_ST, BUZZER_NOTE_ST, BUZZER_NOTE_ST,
    BUZZER_NOTE_ST, BUZZER_NOTE_ST, BUZZER_NOTE_ST, BUZZER_NOTE_ST,
};

static uint32_t led_color[LED_WS2812_NR] = { 0 };

static led_effect_t led_effect = LED_EFFECT_FADE;

static led_fade_state_t led_fade_state = LED_FADE_1;
static led_fade_dir_t led_fade_dir = LED_FADE_UP;
static led_fade_speed_t led_fade_speed = LED_FADE_SLOW;
static int32_t led_fade_bright = 0;

/** Prototypes ---------------------------------------------------- */
static void led_update(void);
static bool led_effect_fade(void);

static void timer_callback(void);
static bool timer_check_timeout(uint32_t timeshot, uint32_t timeout);

/** Internal functions -------------------------------------------- */
/**
 * @brief Updates the LED strip.
 */
static void led_update(void) {
    memset(led_color, 0, sizeof(led_color));

    switch (led_effect) {
        case LED_EFFECT_FADE: {
            if (led_effect_fade() == true) {
            }
            break;
        }
        default: {
            break;
        }
    }

    led_ws2812_write(led_color, LED_WS2812_NR);
}

/**
 * @brief Updates LED strip with fade effect.
 * 
 * @return true when the effect is finished
 */
static bool led_effect_fade(void) {
    bool effect_finish = false;
    uint32_t bright_step = 0;
    uint32_t i = 0;

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

    if (led_fade_dir == LED_FADE_UP) {
        led_fade_bright += bright_step;
        if (led_fade_bright > LED_WS2812_COLOR_MAX) {
            led_fade_bright = LED_WS2812_COLOR_MAX;
            led_fade_dir = LED_FADE_DOWN;
        }
    } else {
        led_fade_bright -= bright_step;
        if (led_fade_bright < 0) {
            led_fade_bright = 0;
            led_fade_dir = LED_FADE_UP;

            led_fade_state++;
            if (led_fade_state > LED_FADE_5) {
                led_fade_state = LED_FADE_1;

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
 * @brief Timer callback.
 * 
 * Executed every 100us.
 */
static void timer_callback(void) {
    timer_counter++;

    infrared_update();
}

static bool timer_check_timeout(uint32_t timeshot, uint32_t timeout) {
    volatile uint32_t time_diff = timer_counter - timeshot;

    if (time_diff >= timeout) {
        return true;
    }

    return false;
}

/** Public functions ---------------------------------------------- */
int main(void) {
    uint32_t led_update_timeshot = 0;
    uint32_t buzzer_timeshot = 0;
    uint32_t note_idx = 0;

    rcc_clock_init();

    buzzer_setup();
    infrared_setup();
    led_ws2812_setup();

    timer_setup(TIMER_1, 71, 99);
    timer_attach_callback(TIMER_1, timer_callback);

    while (true) {
        if (timer_check_timeout(led_update_timeshot, LED_UPDATE_TIME) == true) {
            led_update_timeshot = timer_counter;

            led_update();
        }

        if (timer_check_timeout(buzzer_timeshot, BUZZER_NOTE_TIME) == true) {
            buzzer_timeshot = timer_counter;

            buzzer_play_note(sheet_music[note_idx]);

            note_idx++;
            if (note_idx >= (sizeof(sheet_music) / sizeof(sheet_music[0]))) {
                note_idx = 0;
            }
        }
    }
}
