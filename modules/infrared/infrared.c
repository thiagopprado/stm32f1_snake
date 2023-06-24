/**
 * Author: thiagopereiraprado@gmail.com
 * 
 * @brief Infrared module implementation.
 * 
 */
#include "infrared.h"

#include <string.h>
#include <stdbool.h>

#define BIT_CHECK(value, bit)       ((value >> bit) & 0x01)
#define BIT_SET(value, bit)         (value |= 1 << bit)
#define BIT_CLEAR(value, bit)       (value &= ~(1 << bit))

/**
 * @brief State machine timings, for 100us base.
 * @{
 */
/** NEC */
#define IR_NEC_START_TIMEOUT        200
#define IR_NEC_BIT_BURST_TIMEOUT    10
#define IR_NEC_BIT_LOW_TIMEOUT      10
#define IR_NEC_BIT_TIMEOUT          30
#define IR_NEC_BIT_NR               32

/** RC6 */
#define IR_RC6_START_1_TIMEOUT      30
#define IR_RC6_START_2_TIMEOUT      15
#define IR_RC6_TOGGLE_1_TIMEOUT     30
#define IR_RC6_TOGGLE_2_TIMEOUT     10
#define IR_RC6_HALF_BIT_TIMEOUT     5
#define IR_RC6_BIT_TIMEOUT          10
#define IR_RC6_FIELD_BIT_NR         3
#define IR_RC6_BIT_NR               16
/** @} */

/**
 * @brief Remote control codes.
 * @{
 */
/** NEC */
#define IR_NEC_CODE_ENTER           0x97680707
#define IR_NEC_CODE_ESC             0xa7580707
#define IR_NEC_CODE_UP              0x9f600707
#define IR_NEC_CODE_DOWN            0x9e610707
#define IR_NEC_CODE_LEFT            0x9a650707
#define IR_NEC_CODE_RIGHT           0x9d620707

/** RC6 */
#define IR_RC6_CODE_ENTER_1         0x3bff
#define IR_RC6_CODE_ESC_1           0x5fff
#define IR_RC6_CODE_UP_1            0x1bff
#define IR_RC6_CODE_DOWN_1          0x9bff
#define IR_RC6_CODE_LEFT_1          0x5bff
#define IR_RC6_CODE_RIGHT_1         0xdbff

#define IR_RC6_CODE_ENTER_2         0x3a00
#define IR_RC6_CODE_ESC_2           0x5000
#define IR_RC6_CODE_UP_2            0x1a00
#define IR_RC6_CODE_DOWN_2          0x9a00
#define IR_RC6_CODE_LEFT_2          0x5a00
#define IR_RC6_CODE_RIGHT_2         0xda00
/** @} */

typedef enum {
    IR_NEC_STATE_INIT = 0,
    IR_NEC_STATE_START,
    IR_NEC_STATE_READ_WAIT,
    IR_NEC_STATE_READ_GET,
    IR_NEC_STATE_STOP,
} infrared_nec_state_t;

typedef enum {
    IR_RC6_STATE_INIT = 0,
    IR_RC6_STATE_START_1,
    IR_RC6_STATE_START_2,
    IR_RC6_STATE_START_3,
    IR_RC6_STATE_READ_FIELD_WAIT,
    IR_RC6_STATE_READ_TOGGLE_1,
    IR_RC6_STATE_READ_TOGGLE_2,
    IR_RC6_STATE_READ_GET,
} infrared_rc6_state_t;

typedef struct {
    infrared_nec_state_t state;
    uint32_t value;
    uint32_t new_value;
    uint16_t read_counter;
    uint8_t bit_idx;
    bool last_pin_value;
} infrared_nec_ctrl_t;

typedef struct {
    infrared_rc6_state_t state;
    uint16_t value;
    uint16_t new_value;
    uint8_t bit_idx;
    uint8_t field_bits_nr;
    bool transition_ctrl;
    uint16_t last_timeshot;
} infrared_rc6_ctrl_t;

static infrared_nec_ctrl_t nec_ctrl;
static infrared_rc6_ctrl_t rc6_ctrl;

static void infrared_nec_read(uint16_t timeshot);
static void infrared_rc6_read(uint16_t timeshot);
static void infrared_input_capture_callback(void);

/**
 * @brief NEC state machine.
 * 
 * @param timeshot  Timestamp of the pin change.
 */
static void infrared_nec_read(uint16_t timeshot) {
    bool new_pin_value = gpio_read(INFRARED_PORT, INFRARED_PIN);

    // TODO: Update based on input capture edge detection
    nec_ctrl.read_counter++;

    switch (nec_ctrl.state) {
        case IR_NEC_STATE_INIT: {
            if (nec_ctrl.last_pin_value == true && new_pin_value == false) {
                nec_ctrl.state = IR_NEC_STATE_START;
                nec_ctrl.read_counter = 0;
            }

            break;
        }
        case IR_NEC_STATE_START: {
            if (nec_ctrl.last_pin_value == true && new_pin_value == false) {
                nec_ctrl.state = IR_NEC_STATE_READ_WAIT;
                nec_ctrl.bit_idx = 0;
                nec_ctrl.read_counter = 0;
                nec_ctrl.new_value = 0;

            } else if (nec_ctrl.read_counter > IR_NEC_START_TIMEOUT) {
                nec_ctrl.state = IR_NEC_STATE_INIT;
                nec_ctrl.read_counter = 0;
            }

            break;
        }
        case IR_NEC_STATE_READ_WAIT: {
            if (new_pin_value == true) {
                nec_ctrl.state = IR_NEC_STATE_READ_GET;
                nec_ctrl.read_counter = 0;

            } else if (nec_ctrl.read_counter > IR_NEC_BIT_BURST_TIMEOUT) {
                // Invalid bit
                nec_ctrl.state = IR_NEC_STATE_INIT;
            }

            break;
        }
        case IR_NEC_STATE_READ_GET: {
            if (new_pin_value == false) {
                if (nec_ctrl.read_counter <= IR_NEC_BIT_LOW_TIMEOUT) {
                	BIT_CLEAR(nec_ctrl.new_value, nec_ctrl.bit_idx);
                } else {
                    BIT_SET(nec_ctrl.new_value, nec_ctrl.bit_idx);
                }

                nec_ctrl.bit_idx++;
                if (nec_ctrl.bit_idx == IR_NEC_BIT_NR) {
                    nec_ctrl.state = IR_NEC_STATE_STOP;

                } else {
                    nec_ctrl.state = IR_NEC_STATE_READ_WAIT;
                }

                nec_ctrl.read_counter = 0;

            } else if (nec_ctrl.read_counter > IR_NEC_BIT_TIMEOUT) {
                // Invalid bit
                nec_ctrl.state = IR_NEC_STATE_INIT;
            }

            break;
        }
        case IR_NEC_STATE_STOP: {
            if (new_pin_value == true) {
                nec_ctrl.state = IR_NEC_STATE_INIT;
                nec_ctrl.value = nec_ctrl.new_value;

            } else if (nec_ctrl.read_counter > IR_NEC_BIT_BURST_TIMEOUT) {
                // Invalid bit
                nec_ctrl.state = IR_NEC_STATE_INIT;
            }

            break;
        }
        default: {
            break;
        }
    }

    nec_ctrl.last_pin_value = new_pin_value;
}

/**
 * @brief RC6 state machine.
 * 
 * @param timeshot  Timestamp of the pin change.
 */
static void infrared_rc6_read(uint16_t timeshot) {
    uint16_t time_interval = timeshot - rc6_ctrl.last_timeshot;
    bool pin_value = gpio_read(INFRARED_PORT, INFRARED_PIN);

    rc6_ctrl.last_timeshot = timeshot;

    if (time_interval > 100 && rc6_ctrl.state != IR_RC6_STATE_INIT) {
        rc6_ctrl.state = IR_RC6_STATE_INIT;
    }

    switch (rc6_ctrl.state) {
        case IR_RC6_STATE_INIT: {
            if (pin_value == false) {
                rc6_ctrl.state = IR_RC6_STATE_START_1;
            }

            break;
        }
        case IR_RC6_STATE_START_1: {
            if (time_interval > IR_RC6_START_1_TIMEOUT) {
                // Invalid start
                rc6_ctrl.state = IR_RC6_STATE_INIT;
            } else if (pin_value == true) {
                rc6_ctrl.state = IR_RC6_STATE_START_2;
            }

            break;
        }
        case IR_RC6_STATE_START_2: {
            if (time_interval > IR_RC6_START_2_TIMEOUT) {
                // Invalid start
                rc6_ctrl.state = IR_RC6_STATE_INIT;
            } else if (pin_value == false) {
                rc6_ctrl.state = IR_RC6_STATE_START_3;
            }

            break;
        }
        case IR_RC6_STATE_START_3: {
            if (time_interval > IR_RC6_HALF_BIT_TIMEOUT) {
                // Invalid start
                rc6_ctrl.state = IR_RC6_STATE_INIT;
            } else if (pin_value == true) {
                rc6_ctrl.state = IR_RC6_STATE_READ_FIELD_WAIT;
                rc6_ctrl.transition_ctrl = false;
            }

            break;
        }
        case IR_RC6_STATE_READ_FIELD_WAIT: {
            if (time_interval > IR_RC6_BIT_TIMEOUT) {
                // Invalid field
                rc6_ctrl.state = IR_RC6_STATE_INIT;
                rc6_ctrl.field_bits_nr = 0;

            } else if (time_interval > IR_RC6_HALF_BIT_TIMEOUT || rc6_ctrl.transition_ctrl == true) {
                rc6_ctrl.field_bits_nr++;
                if (rc6_ctrl.field_bits_nr == IR_RC6_FIELD_BIT_NR) {
                    rc6_ctrl.field_bits_nr = 0;
                    rc6_ctrl.state = IR_RC6_STATE_READ_TOGGLE_1;
                }
                rc6_ctrl.transition_ctrl = false;

            } else if (rc6_ctrl.transition_ctrl == false) {
                rc6_ctrl.transition_ctrl = true;
            }

            break;
        }
        case IR_RC6_STATE_READ_TOGGLE_1: {
            if (time_interval > IR_RC6_TOGGLE_1_TIMEOUT) {
                // Invalid toggle
                rc6_ctrl.state = IR_RC6_STATE_INIT;
                rc6_ctrl.transition_ctrl = false;
            } else if (time_interval <= IR_RC6_HALF_BIT_TIMEOUT && rc6_ctrl.transition_ctrl == false) {
                rc6_ctrl.transition_ctrl = true;
            } else {
                rc6_ctrl.state = IR_RC6_STATE_READ_TOGGLE_2;
            }

            break;
        }
        case IR_RC6_STATE_READ_TOGGLE_2: {
            rc6_ctrl.state = IR_RC6_STATE_READ_GET;
            rc6_ctrl.bit_idx = 0;
            rc6_ctrl.new_value = 0;
            if (time_interval < IR_RC6_TOGGLE_2_TIMEOUT) {
                break;
            } else {
                time_interval = IR_RC6_BIT_TIMEOUT;
            }
        }
        case IR_RC6_STATE_READ_GET: {
            if (time_interval > IR_RC6_BIT_TIMEOUT) {
                // Invalid bit
                rc6_ctrl.state = IR_RC6_STATE_INIT;
                rc6_ctrl.transition_ctrl = false;
            } else if (time_interval <= IR_RC6_HALF_BIT_TIMEOUT && rc6_ctrl.transition_ctrl == false && rc6_ctrl.bit_idx != 0) {
                rc6_ctrl.transition_ctrl = true;
            } else {
                if (pin_value == true) {
                    BIT_SET(rc6_ctrl.new_value, rc6_ctrl.bit_idx);
                }

                rc6_ctrl.bit_idx++;
                if (rc6_ctrl.bit_idx == IR_RC6_BIT_NR) {
                    rc6_ctrl.bit_idx = 0;
                    rc6_ctrl.state = IR_RC6_STATE_INIT;
                    rc6_ctrl.value = rc6_ctrl.new_value;
                }

                rc6_ctrl.transition_ctrl = false;
            }

            break;
        }
        default: {
            break;
        }
    }
}

/**
 * @brief Input Capture callback.
 */
static void infrared_input_capture_callback(void) {
    TIM_TypeDef *timer_ptr = timer_get_ptr(INFRARED_TIMER);

    timer_ptr->CCER ^= TIM_CCER_CC1P; // Invert polarity
    infrared_nec_read(timer_ptr->CCR1);
    infrared_rc6_read(timer_ptr->CCR1);
}

/**
 * @brief Sets up infrared pin and timer.
 */
void infrared_setup(void) {
    memset(&nec_ctrl, 0, sizeof(infrared_nec_ctrl_t));
    memset(&rc6_ctrl, 0, sizeof(infrared_rc6_ctrl_t));

    gpio_setup(INFRARED_PORT, INFRARED_PIN, GPIO_MODE_INPUT, GPIO_CFG_IN_FLOAT);
    timer_setup(INFRARED_TIMER, 7199, 0xFFFF);
    timer_input_capture_setup(INFRARED_TIMER, INFRARED_IC_CH);
    timer_attach_input_capture_callback(INFRARED_TIMER, INFRARED_IC_CH, infrared_input_capture_callback);
}

/**
 * @brief Gets the value read for NEC protocol.
 * 
 * @return Value read.
 */
uint32_t infrared_read_nec(void)
{
    uint32_t read_value = nec_ctrl.value;

    nec_ctrl.value = 0;

    return read_value;
}

/**
 * @brief Gets the value read for RC6 protocol.
 * 
 * @return Value read.
 */
uint32_t infrared_read_rc6(void)
{
    uint16_t read_value = rc6_ctrl.value;

    rc6_ctrl.value = 0;

    return read_value;
}

/**
 * @brief Decodes NEC and RC6 read values into a key code.
 * 
 * @return Key pressed.
 */
ir_key_id_t infrared_decode(void) {
    ir_key_id_t key_pressed = INFRARED_KEY_NONE;
    uint32_t nec_read = infrared_read_nec();
    uint16_t rc6_read = infrared_read_rc6();

    if (nec_read == IR_NEC_CODE_ENTER || rc6_read == IR_RC6_CODE_ENTER_1 || rc6_read == IR_RC6_CODE_ENTER_2) {
        key_pressed = INFRARED_KEY_ENTER;
    } else if (nec_read == IR_NEC_CODE_ESC || rc6_read == IR_RC6_CODE_ESC_1 || rc6_read == IR_RC6_CODE_ESC_2) {
        key_pressed = INFRARED_KEY_ESC;
    } else if (nec_read == IR_NEC_CODE_UP || rc6_read == IR_RC6_CODE_UP_1 || rc6_read == IR_RC6_CODE_UP_2) {
        key_pressed = INFRARED_KEY_UP;
    } else if (nec_read == IR_NEC_CODE_DOWN || rc6_read == IR_RC6_CODE_DOWN_1 || rc6_read == IR_RC6_CODE_DOWN_2) {
        key_pressed = INFRARED_KEY_DOWN;
    } else if (nec_read == IR_NEC_CODE_LEFT || rc6_read == IR_RC6_CODE_LEFT_1 || rc6_read == IR_RC6_CODE_LEFT_2) {
        key_pressed = INFRARED_KEY_LEFT;
    } else if (nec_read == IR_NEC_CODE_RIGHT || rc6_read == IR_RC6_CODE_RIGHT_1 || rc6_read == IR_RC6_CODE_RIGHT_2) {
        key_pressed = INFRARED_KEY_RIGHT;
    }

    return key_pressed;
}
