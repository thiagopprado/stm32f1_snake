#include <stdint.h>
#include <stdbool.h>

#include "stm32f1xx.h"
#include "core_cm3.h"

#include "rcc.h"

/** Definitions --------------------------------------------------- */

/** Types --------------------------------------------------------- */

/** Variables ----------------------------------------------------- */

/** Prototypes ---------------------------------------------------- */

/** Internal functions -------------------------------------------- */

/** Public functions ---------------------------------------------- */
int main(void) {
    rcc_clock_init();

    while (true) {
    }
}
