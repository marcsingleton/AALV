#include <stdarg.h>
#include <stdio.h>

#include "error.h"

char error_message[ERROR_MESSAGE_LEN];

void error_printf(const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    vsnprintf(error_message, ERROR_MESSAGE_LEN, format, ap);
    va_end(ap);
}
