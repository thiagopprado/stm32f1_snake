/**
 * Author: thiagopereiraprado@gmail.com
 * 
 * @file
 * @defgroup snake Snake game
 * @brief Snake game
 * 
 */
#ifndef SNAKE_H
#define SNAKE_H

#include <stdint.h>

/**
 * @ingroup snake
 * @brief Moving directions.
 */
typedef enum {
    SNAKE_DIR_RIGHT = 0,    /**< Snake moving right. */
    SNAKE_DIR_DOWN,         /**< Snake moving down. */
    SNAKE_DIR_LEFT,         /**< Snake moving left. */
    SNAKE_DIR_UP,           /**< Snake moving up. */
} snake_dir_t;

/**
 * @ingroup snake
 * @brief Keyboard keys.
 */
typedef enum {
    SNAKE_KEY_NONE = 0, /**< No key pressed. */
    SNAKE_KEY_RIGHT,    /**< Right pressed. */
    SNAKE_KEY_DOWN,     /**< Down pressed. */
    SNAKE_KEY_LEFT,     /**< Left pressed. */
    SNAKE_KEY_UP,       /**< Up pressed. */
} snake_key_t;

/**
 * @ingroup snake
 * @brief Game states.
 */
typedef enum {
    SNAKE_STATE_PLAYING = 0,  /**< Playing the game. */
    SNAKE_STATE_GAME_OVER,    /**< Game over! */
    SNAKE_STATE_WIN,          /**< You win! */
} snake_state_t;

/**
 * @ingroup snake
 * @brief Collision state.
 */
typedef enum {
    SNAKE_COLLISION_FALSE = 0,  /**< The point doesn`t colide with a snake part. */
    SNAKE_COLLISION_TRUE,       /**< The point colides with a snake part. */
} snake_collision_t;

/**
 * @ingroup snake
 * @brief Game coordinates.
 *
 * The game coordinates must be inside the following range:
 * x = (0, 20), y = (0, 11)
 */
typedef struct {
    uint8_t x;  /**< X coordinate (collumn). */
    uint8_t y;  /**< Y coordinate (line). */
} snake_pos_t;

void snake_init(void);
void snake_update(void);

#endif /* SNAKE_H */
