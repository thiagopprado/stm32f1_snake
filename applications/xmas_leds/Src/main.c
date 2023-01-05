#include <stdint.h>
#include <stdbool.h>

#include "rcc.h"
#include "timer.h"

/** Definitions --------------------------------------------------- */

/** Types --------------------------------------------------------- */

/** Variables ----------------------------------------------------- */
static volatile uint32_t timer_counter = 0;

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
    rcc_clock_init();

    timer_setup(TIMER_1, 71, 99);
    timer_attach_callback(TIMER_1, timer_callback);

    while (true) {
    }
}
