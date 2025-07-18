#ifndef TERMINAL_H
#define TERMINAL_H

#include <termios.h>
#include <unistd.h>

extern int TERMINAL_FILENO;

typedef enum
{
    FG_BLACK = 30,
    FG_RED,
    FG_GREEN,
    FG_YELLOW,
    FG_BLUE,
    FG_MAGENTA,
    FG_CYAN,
    FG_WHITE,
    BG_BLACK = 40,
    BG_RED,
    BG_GREEN,
    BG_YELLOW,
    BG_BLUE,
    BG_MAGENTA,
    BG_CYAN,
    BG_WHITE,
    FG_BRIGHT_BLACK = 90,
    FG_BRIGHT_RED,
    FG_BRIGHT_GREEN,
    FG_BRIGHT_YELLOW,
    FG_BRIGHT_BLUE,
    FG_BRIGHT_MAGENTA,
    FG_BRIGHT_CYAN,
    FG_BRIGHT_WHITE,
    BG_BRIGHT_BLACK = 100,
    BG_BRIGHT_RED,
    BG_BRIGHT_GREEN,
    BG_BRIGHT_YELLOW,
    BG_BRIGHT_BLUE,
    BG_BRIGHT_MAGENTA,
    BG_BRIGHT_CYAN,
    BG_BRIGHT_WHITE,
} Color4Bit;

typedef uint8_t Color8Bit;

int terminal_get_termios(struct termios *termios_p);
int terminal_get_window_size(unsigned int *rows, unsigned int *cols);
int terminal_enable_raw_mode(struct termios *old_termios, struct termios *raw_termios);
int terminal_disable_raw_mode(struct termios *old_termios);
void terminal_use_alternate_buffer(void);
void terminal_use_normal_buffer(void);
void terminal_cursor_up(Array *buffer);
void terminal_cursor_down(Array *buffer);
void terminal_cursor_right(Array *buffer);
void terminal_cursor_left(Array *buffer);
void terminal_cursor_origin(Array *buffer);
void terminal_cursor_ij(Array *buffer, const unsigned int i, const unsigned int j);
void terminal_cursor_hide(Array *buffer);
void terminal_cursor_show(Array *buffer);
void terminal_clear_screen(Array *buffer);
void terminal_clear_line(Array *buffer);
void terminal_clear_line_left(Array *buffer);
void terminal_clear_line_right(Array *buffer);
void terminal_set_foreground_color_4bit(Array *buffer, const Color4Bit color);
void terminal_set_background_color_4bit(Array *buffer, const Color4Bit color);
void terminal_set_color_4bit(Array *buffer, const Color4Bit fg_color, const Color4Bit bg_color);
void terminal_set_foreground_color_8bit(Array *buffer, const Color8Bit color);
void terminal_set_background_color_8bit(Array *buffer, const Color8Bit color);
void terminal_set_color_8bit(Array *buffer, const Color8Bit fg_color, const Color8Bit bg_color);
void terminal_set_foreground_color_default(Array *buffer);
void terminal_set_background_color_default(Array *buffer);
void terminal_set_color_default(Array *buffer);

#endif // TERMINAL_H
