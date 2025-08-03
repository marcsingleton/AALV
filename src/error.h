#ifndef ERROR_H
#define ERROR_H

/*
 * Error handling
 */

#define ERROR_MESSAGE_LEN 256

extern char error_message[];

void error_printf(const char *format, ...);

#endif // ERROR_H
