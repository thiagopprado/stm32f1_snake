#ifndef SNAKE_H
#define SNAKE_H

#include <stdint.h>

typedef enum {
    SNAKE_DIR_RIGHT = 0,
    SNAKE_DIR_DOWN,
    SNAKE_DIR_LEFT,
    SNAKE_DIR_UP,
} snake_dir_t;

typedef struct {
	uint8_t x;
	uint8_t y;
} snake_pos_t;

typedef enum {
    SNAKE_KEY_NONE = 0,
    SNAKE_KEY_RIGHT,
    SNAKE_KEY_DOWN,
    SNAKE_KEY_LEFT,
    SNAKE_KEY_UP,
} snake_key_t;

typedef enum {
    SNAKE_STATE_PLAYING = 0,
    SNAKE_STATE_GAME_OVER,
    SNAKE_STATE_WIN,
} snake_state_t;

typedef enum {
    SNAKE_COLLISION_FALSE = 0,
    SNAKE_COLLISION_TRUE,
} snake_collision_t;


void snake_init(void);
void snake_update(void);
void snake_kbd_debounce(void);

#endif /* SNAKE_H */
