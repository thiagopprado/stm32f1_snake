/**
 * Author: thiagopereiraprado@gmail.com
 * 
 * @file
 * @defgroup nokia5110 Nokia 5110 display driver
 * @brief Nokia 5110 display driver
 * 
 */
#ifndef NOKIA5110_H
#define NOKIA5110_H

#include <stdint.h>

#define NOKIA5110_MAX_LINE_NR   6
#define NOKIA5110_MAX_COL_NR    84
#define NOKIA5110_BYTES_NR      504

void nokia5110_setup(void);
void nokia5110_move_cursor(uint8_t x, uint8_t y);
void nokia5110_clear_screen(void);
void nokia5110_char(char character);
void nokia5110_char_at(char character, uint8_t x, uint8_t y);
void nokia5110_string(char* string);
void nokia5110_string_at(char* string, uint8_t x, uint8_t y);
void nokia5110_update_screen(void);
void nokia5110_clear_buffer(void);
void nokia5110_set_pixel(uint8_t x, uint8_t y);
void nokia5110_clr_pixel(uint8_t x, uint8_t y);
void nokia5110_draw_rectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);
void nokia5110_clear_rectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);

#endif /* NOKIA5110_H */
