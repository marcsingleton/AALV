#include <stdio.h>
#include <string.h>

#include "linenoise.h"

#include "cmd.h"
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

    int retcode;

    retcode = strcmp(command, "q");
    if (retcode == 0)
        exit(0);

    retcode = strcmp(command, "n");
    if (retcode == 0)
    {
        cmd_next_file();
        return 0;
    }

    retcode = strcmp(command, "p");
    if (retcode == 0)
    {
        cmd_previous_file();
        return 0;
    }

    unsigned int tick_offset;
    retcode = sscanf(command, "set tick_offset %u", &tick_offset);
    if (retcode == 1)
    {
        state_set_tick_offset(&state, tick_offset);
        return 0;
    }

    return 1;
}

void cmd_next_file(void)
{
    if (state.active_file_index + 1 >= state.nfiles)
        return;
    state_set_active_file_index(&state, state.active_file_index + 1);
    state.refresh_window = true;
}

void cmd_previous_file(void)
{
    if (state.active_file_index == 0)
        return;
    state_set_active_file_index(&state, state.active_file_index - 1);
    state.refresh_window = true;
}
