#ifndef FORMATS_H
#define FORMATS_H

/*
 * File format interface
 */

#include <sys/types.h>

typedef enum
{
    FORMATS_ERROR_SUCCESS = 0,
    FORMATS_ERROR_INVALID_FORMAT = -1,
    FORMATS_ERROR_PARSING = -2,
    FORMATS_ERROR_RECORD_OVERFLOW = -3,
    FORMATS_ERROR_SEQUENCE_OVERFLOW = -4,
    FORMATS_ERROR_FILE_IO = -5,
    FORMATS_ERROR_MEMORY_ALLOCATION = -6,
} FormatsError;

int formats_line_is_empty(char *line, ssize_t line_len);

#endif // FORMATS_H