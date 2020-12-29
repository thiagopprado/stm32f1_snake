/**
 * Author: thiagopereiraprado@gmail.com
 * 
 * @file
 * @ingroup snake
 * @brief Snake game implementation
 * 
 */
#include "snake.h"

#include "nokia5110.h"
#include "gpio.h"

#include <stdlib.h>

// Define as 1 to draw the snake with 3 pixels width
#define SNAKE_THINNER   0

// Rectangle coordinates (in pixels)
#define SNAKE_RECT_X1   0
#define SNAKE_RECT_Y1   0
#define SNAKE_RECT_X2   83
#define SNAKE_RECT_Y2   47

// Each snake part has 4 pixels
#define SNAKE_PART_SIZE   4

#define SNAKE_MAX_SIZE  220
#define SNAKE_MAX_X     20
#define SNAKE_MAX_Y     11

#define SNAKE_INIT_FOOD_X   10
#define SNAKE_INIT_FOOD_Y   5
#define SNAKE_INIT_SIZE     3

// Initial available pixels for the game
#define SNAKE_X_0       2
#define SNAKE_Y_0       2

#define SNAKE_DEBOUNCE_CNT  10

// Snake and food coordinates
snake_pos_t snake[SNAKE_MAX_SIZE] = { {2, 0}, {1, 0}, {0, 0}, };
snake_pos_t food = {SNAKE_INIT_FOOD_X, SNAKE_INIT_FOOD_Y};
snake_state_t game_state = SNAKE_STATE_PLAYING;
snake_dir_t direction = SNAKE_DIR_RIGHT;
snake_dir_t last_direction = SNAKE_DIR_RIGHT;
snake_key_t key_pressed = SNAKE_KEY_NONE;
uint8_t size = SNAKE_INIT_SIZE;
uint8_t head = 0;

snake_collision_t snake_check_collision(snake_pos_t position);
void snake_draw_part(snake_pos_t part_coord);
void snake_erase_part(snake_pos_t part_coord);
void snake_draw_food(void);

/**
 * @ingroup snake
 * @brief Inits the snake game
 *
 * Sets up the keyboard, resets the game parameters and draw 
 * the initial food and snake.
 *
 * The available pixels for the game are within th following range:
 * x = (2, 81) and y = (2, 45).
 *
 * Each snake part has 4 pixels, so dividing it, the game has 20
 * horizontal and 11 vertical spaces available.
 *
 * @note The function @ref nokia5110_update_screen must be called
 * to actually update the screen.
 */
void snake_init(void) {
    uint8_t i = 0;

    // Cfg keyboard
    gpio_setup(GPIO_PORTB, 12, GPIO_MODE_INPUT, GPIO_CFG_IN_PULL); // Right
    gpio_write(GPIO_PORTB, 12, GPIO_STATE_HIGH); // Pull up

    gpio_setup(GPIO_PORTB, 13, GPIO_MODE_INPUT, GPIO_CFG_IN_PULL); // Down
    gpio_write(GPIO_PORTB, 13, GPIO_STATE_HIGH); // Pull up

    gpio_setup(GPIO_PORTB, 14, GPIO_MODE_INPUT, GPIO_CFG_IN_PULL); // Left
    gpio_write(GPIO_PORTB, 14, GPIO_STATE_HIGH); // Pull up

    gpio_setup(GPIO_PORTB, 15, GPIO_MODE_INPUT, GPIO_CFG_IN_PULL); // Up
    gpio_write(GPIO_PORTB, 15, GPIO_STATE_HIGH); // Pull up

    // Draw game borders
    nokia5110_clear_buffer();
    nokia5110_draw_rectangle(SNAKE_RECT_X1, SNAKE_RECT_Y1, SNAKE_RECT_X2, SNAKE_RECT_Y2);
    
    // Initial position
    snake[0].x = 2;
    snake[0].y = 0;

    snake[1].x = 1;
    snake[1].y = 0;

    snake[2].x = 0;
    snake[2].y = 0;

    food.x = SNAKE_INIT_FOOD_X;
    food.y = SNAKE_INIT_FOOD_Y;

    game_state = SNAKE_STATE_PLAYING;
    direction = SNAKE_DIR_RIGHT;
    key_pressed = SNAKE_KEY_NONE;
    size = SNAKE_INIT_SIZE;
    head = 0;

    // Draw init snake and food
    for (i = 0; i < size; i++) {
        snake_draw_part(snake[i]);
    }
    snake_draw_food();
}

/**
 * @ingroup snake
 * @brief Updates the snake game
 *
 * Recalculates the direction based on the key pressed and moves the
 * snake head according to it.
 * Checks if the new head is inside the snake itself, changing the
 * game state to game over, and if it's equal the food coordinates,
 * increasing snake size and drawing the next food.
 *
 * @note The function @ref nokia5110_update_screen must be called
 * to actually update the screen.
 */
void snake_update(void) {
    uint16_t tail = head + size - 1;
    uint8_t new_head = head - 1;

    // If game over, waits for input to reset
    if (game_state == SNAKE_STATE_GAME_OVER) {
        if (key_pressed != SNAKE_KEY_NONE) {
            snake_init();
        }
        return;
    }

    if (head == 0) {
        // Last array position (circular buffer)
        new_head = SNAKE_MAX_SIZE - 1;
    }

    if (tail >= SNAKE_MAX_SIZE) {
        tail -= SNAKE_MAX_SIZE;
    }

    last_direction = direction;

    // Updates direction
    switch (key_pressed) {
        case SNAKE_KEY_RIGHT:
            if (direction != SNAKE_DIR_LEFT) {
                direction = SNAKE_DIR_RIGHT;
            }
        break;
        case SNAKE_KEY_DOWN:
            if (direction != SNAKE_DIR_UP) {
                direction = SNAKE_DIR_DOWN;
            }
        break;
        case SNAKE_KEY_LEFT:
            if (direction != SNAKE_DIR_RIGHT) {
                direction = SNAKE_DIR_LEFT;
            }
        break;
        case SNAKE_KEY_UP:
            if (direction != SNAKE_DIR_DOWN) {
                direction = SNAKE_DIR_UP;
            }
        break;
        default:
        // Keep direction
        break;
    }

    // Calculates the new head
    switch (direction) {
        case SNAKE_DIR_RIGHT:
            snake[new_head].y = snake[head].y;
            snake[new_head].x = snake[head].x + 1;
            if (snake[new_head].x == SNAKE_MAX_X) {
                snake[new_head].x = 0;
            }
        break;
        case SNAKE_DIR_DOWN:
            snake[new_head].x = snake[head].x;
            snake[new_head].y = snake[head].y + 1;
            if (snake[new_head].y == SNAKE_MAX_Y) {
                snake[new_head].y = 0;
            }
        break;
        case SNAKE_DIR_LEFT:
            snake[new_head].y = snake[head].y;
            snake[new_head].x = snake[head].x - 1;
            if (snake[head].x == 0) {
                snake[new_head].x = SNAKE_MAX_X - 1;
            }
        break;
        case SNAKE_DIR_UP:
            snake[new_head].x = snake[head].x;
            snake[new_head].y = snake[head].y - 1;
            if (snake[head].y == 0) {
                snake[new_head].y = SNAKE_MAX_Y - 1;
            }
        break;
    }
    
    // Checks collision
    if (snake_check_collision(snake[new_head]) == SNAKE_COLLISION_TRUE) {
        // New head hitted a snake part
        nokia5110_string_at(" Game Over! ", 6, 2);
        nokia5110_string_at(" Score:     ", 6, 3);
        nokia5110_char_at('0' + (size / 100), 52, 3);
        nokia5110_char('0' + ((size / 10) % 10));
        nokia5110_char('0' + (size % 10));
        game_state = SNAKE_STATE_GAME_OVER;
        key_pressed = SNAKE_KEY_NONE;
        return;
    }

    // Prints new head
    head = new_head;
    snake_draw_part(snake[head]);

    // Checks if new head reached the food
    if ((snake[new_head].x == food.x) &&
        (snake[new_head].y == food.y)) {
        size++;
        // Calculates new food position
        do {
            food.x = rand() % SNAKE_MAX_X;
            food.y = rand() % SNAKE_MAX_Y;
        } while (snake_check_collision(food) == SNAKE_COLLISION_TRUE);
        snake_draw_food();
    } else {
        // Erases tail only if didn't reached the food
        snake_erase_part(snake[tail]);
    }
}

/**
 * @ingroup snake
 * @brief Reads the keyboard and debounces the keys.
 *
 * Only updates the key_pressed varible after the keys' state
 * keeps stable for 10 cycles.
 */
void snake_kbd_debounce(void) {
    static snake_key_t previous_key = SNAKE_KEY_NONE;
    static uint8_t debounce_counter = SNAKE_DEBOUNCE_CNT;
    snake_key_t current_key = SNAKE_KEY_NONE;

    if (gpio_read(GPIO_PORTB, 12) == GPIO_STATE_LOW) {
        current_key = SNAKE_KEY_RIGHT;
    }
    if (gpio_read(GPIO_PORTB, 13) == GPIO_STATE_LOW) {
        current_key = SNAKE_KEY_DOWN;
    }
    if (gpio_read(GPIO_PORTB, 14) == GPIO_STATE_LOW) {
        current_key = SNAKE_KEY_LEFT;
    }
    if (gpio_read(GPIO_PORTB, 15) == GPIO_STATE_LOW) {
        current_key = SNAKE_KEY_UP;
    }

    if (previous_key == current_key) {
        debounce_counter--;
        if (debounce_counter == 0) {
            /** Same key pressed for 10 cycles, update the key pressed only if it's
             *  different than none, to let the update function clear the key pressed.
             */
            if (current_key != SNAKE_KEY_NONE) {
                key_pressed = current_key;
            }
            debounce_counter = SNAKE_DEBOUNCE_CNT;
        }
    } else {
        previous_key = current_key;
        debounce_counter = SNAKE_DEBOUNCE_CNT;
    }
}

/**
 * @ingroup snake
 * @brief Checks if the given position is inside the snake.
 *
 * @param position  Coordinate to check.
 *
 * @return TRUE, if the point is inside the snake, FALSE, otherwise.
 */
snake_collision_t snake_check_collision(snake_pos_t position) {
    uint8_t i = 0;
    uint16_t compare_pos = 0;

    // Compare the given position with all snake parts
    for (i = 0; i < size; i++) {
        compare_pos = head + i;
        if (compare_pos >= SNAKE_MAX_SIZE) {
            compare_pos -= SNAKE_MAX_SIZE;
        }

        if ((position.x == snake[compare_pos].x) &&
            (position.y == snake[compare_pos].y)) {
            return SNAKE_COLLISION_TRUE;
        }
    }

    return SNAKE_COLLISION_FALSE;
}

/**
 * @ingroup snake
 * @brief Draw the food
 */
void snake_draw_food(void) {
    uint8_t x, y;

    x = SNAKE_X_0 + SNAKE_PART_SIZE * food.x;
    y = SNAKE_Y_0 + SNAKE_PART_SIZE * food.y;

    nokia5110_set_pixel(x + 1, y + 1);
    nokia5110_set_pixel(x + 2, y + 2);
    nokia5110_set_pixel(x + 1, y + 3);
    nokia5110_set_pixel(x, y + 2);
}

/**
 * @ingroup snake
 * @brief Draw a snake part
 *
 * @param part_coord  Coordinates of the part to draw.
 */
void snake_draw_part(snake_pos_t part_coord) {
    uint8_t i, j, x, y;

    x = SNAKE_X_0 + SNAKE_PART_SIZE * part_coord.x;
    y = SNAKE_Y_0 + SNAKE_PART_SIZE * part_coord.y;

    for (i = 0; i < SNAKE_PART_SIZE; i++) {
        for (j = 0; j < SNAKE_PART_SIZE; j++) {
            nokia5110_set_pixel(x + i, y + j);
        }
    }
#if (SNAKE_THINNER == 1)
    // Personalizes the part according to directions
    if (direction == SNAKE_DIR_RIGHT || direction == SNAKE_DIR_LEFT) {
        // Horizontal
        for (i = 0; i < SNAKE_PART_SIZE; i++) {
            nokia5110_clr_pixel(x + i, y);
        }
    } else {
        // Vertical
        for (i = 0; i < SNAKE_PART_SIZE; i++) {
            nokia5110_clr_pixel(x + SNAKE_PART_SIZE - 1, y + i);
        }
    }
    if (last_direction != direction) {
        // Corner
        uint8_t last_head = head + 1, i = 0;

        if (last_head == SNAKE_MAX_SIZE) {
            last_head = 0;
        }

        x = SNAKE_X_0 + SNAKE_PART_SIZE * snake[last_head].x;
        y = SNAKE_Y_0 + SNAKE_PART_SIZE * snake[last_head].y;

        if (last_direction == SNAKE_DIR_UP && direction == SNAKE_DIR_RIGHT) {
            for (i = 0; i < SNAKE_PART_SIZE - 1; i++) {
                nokia5110_clr_pixel(x + i, y);
            }
            for (i = 0; i < SNAKE_PART_SIZE - 1; i++) {
                nokia5110_set_pixel(x + SNAKE_PART_SIZE - 1, y + 1 + i);
            }
        } else if (last_direction == SNAKE_DIR_UP && direction == SNAKE_DIR_LEFT) {
            for (i = 0; i < SNAKE_PART_SIZE - 1; i++) {
                nokia5110_clr_pixel(x + i, y);
            }
        } else if (last_direction == SNAKE_DIR_DOWN && direction == SNAKE_DIR_RIGHT) {
            for (i = 0; i < SNAKE_PART_SIZE - 1; i++) {
                nokia5110_set_pixel(x + SNAKE_PART_SIZE - 1, y + 1 + i);
            }
        } else if (last_direction == SNAKE_DIR_RIGHT && direction == SNAKE_DIR_UP) {
            for (i = 0; i < SNAKE_PART_SIZE - 1; i++) {
                nokia5110_set_pixel(x + i, y);
            }
            for (i = 0; i < SNAKE_PART_SIZE - 1; i++) {
                nokia5110_clr_pixel(x + SNAKE_PART_SIZE - 1, y + 1 + i);
            }
        } else if (last_direction == SNAKE_DIR_RIGHT && direction == SNAKE_DIR_DOWN) {
            for (i = 0; i < SNAKE_PART_SIZE - 1; i++) {
                nokia5110_clr_pixel(x + SNAKE_PART_SIZE - 1, y + 1 + i);
            }
        } else if (last_direction == SNAKE_DIR_LEFT && direction == SNAKE_DIR_UP) {
            for (i = 0; i < SNAKE_PART_SIZE - 1; i++) {
                nokia5110_set_pixel(x + i, y);
            }
        }
    }
#endif /* SNAKE_THINNER */
}

/**
 * @ingroup snake
 * @brief Erase a snake part
 *
 * @param part_coord  Coordinates of the part to erase.
 */
void snake_erase_part(snake_pos_t part_coord) {
    uint8_t i, j, x, y;

    x = SNAKE_X_0 + SNAKE_PART_SIZE * part_coord.x;
    y = SNAKE_Y_0 + SNAKE_PART_SIZE * part_coord.y;

    for (i = 0; i < SNAKE_PART_SIZE; i++) {
        for (j = 0; j < SNAKE_PART_SIZE; j++) {
            nokia5110_clr_pixel(x + i, y + j);
        }
    }
}
