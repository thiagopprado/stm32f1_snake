/** Includes ------------------------------------------------------ */
#include <stdint.h>
#include <stdbool.h>

#include "stm32f1xx.h"
#include "core_cm3.h"

#include "buzzer.h"
#include "rcc.h"
#include "timer.h"

#include "led_strip.h"

/** Definitions --------------------------------------------------- */
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

/** Prototypes ---------------------------------------------------- */
static void timer_callback(void);
static bool timer_check_timeout(uint32_t timeshot, uint32_t timeout);

/** Internal functions -------------------------------------------- */
/**
 * @brief Timer callback.
 * 
 * Executed every 100us.
 */
static void timer_callback(void) {
    timer_counter++;
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

    timer_setup(TIMER_1, 71, 99);
    timer_attach_callback(TIMER_1, timer_callback);

    led_setup();

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
