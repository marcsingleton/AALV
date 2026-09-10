#include "formats.h"

int formats_line_is_empty(char *line, ssize_t line_len)
{
    if (!line)
        return -1;
    if (line_len < 0)
        return -1;

    if (line_len == 0)
        return 1;
    else if (line_len == 1 && line[0] == '\n')
        return 1;
    else if (line_len == 2 && line[0] == '\n' && line[1] == '\r')
        return 1;
    else
        return 0;
}
