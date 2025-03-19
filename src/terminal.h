#ifndef TERMINAL_H
#define TERMINAL_H

#include <termios.h>

void terminal_enable_raw_mode(struct termios *old_termios, struct termios *raw_termios);
void terminal_disable_raw_mode(struct termios *old_termios);

#endif // TERMINAL_H
