#ifndef FORMATS_H
#define FORMATS_H

/*
 * File format interface
 */

#include <sys/types.h>

typedef enum
{
    FORMATS_ENOERR = 0,
    FORMATS_EINVFMT,    // Invalid format
    FORMATS_EPARSE,     // Invalid parser state
    FORMATS_ERECORDMAX, // Maximum number of records
    FORMATS_ESEQMAX,    // Maximum sequence length
    FORMATS_EIO,        // File IO
    FORMATS_ENOMEM,     // Memory allocation
    FORMATS_EWRITE,     // Format write
} FormatsError;

int formats_line_is_empty(char *line, ssize_t line_len);

#endif // FORMATS_H