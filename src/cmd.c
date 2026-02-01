#include <stdio.h>

#include "linenoise.h"

#include "state.h"

extern State state;

char *cmd_read_command(char *prompt)
{
    char *line = linenoise(prompt);
    return line;
}

int cmd_parse_and_execute_command(char *command)
{
    if (!command)
        return 1;

    unsigned int tick_offset;
    int retcode = sscanf(command, "set tick_offset %u", &tick_offset);
    if (retcode > 0)
    {
        state_set_tick_offset(&state, tick_offset);
        return 0;
    }

    return 1;
}
