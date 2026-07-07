#include <stdio.h>
#include <stdlib.h>

#include "cmd.h"
#include "config.h"
#include "macros.h"

unsigned int config_ruler_records_divider;
unsigned int config_header_sequence_divider;
int config_tick_offset;
int config_tick_spacing;
unsigned int config_nucleic_tiebreak_len;

void config_init(void)
{
    config_set_ruler_records_divider(3);
    config_set_header_sequence_divider(30);
    config_set_tick_offset(1);
    config_set_tick_spacing(10);
    config_set_nucleic_tiebreak_len(10);
}

void config_set_ruler_records_divider(unsigned int j)
{
    config_ruler_records_divider = j;
}

void config_set_header_sequence_divider(unsigned int i)
{
    config_header_sequence_divider = i;
}

void config_set_tick_offset(int tick_offset)
{
    config_tick_offset = tick_offset;
}

void config_set_tick_spacing(int tick_spacing)
{
    if (tick_spacing < 1)
        tick_spacing = 1;
    config_tick_spacing = tick_spacing;
}

void config_set_nucleic_tiebreak_len(unsigned int nucleic_tiebreak_len)
{
    config_nucleic_tiebreak_len = nucleic_tiebreak_len;
}

int config_load_user_config(State *state)
{
    char *home_dir = NULL;
    char config_path[128];
    FILE *config_fp = NULL;

    home_dir = getenv("HOME");
    if (!home_dir)
        return -1;

    char *prefixes[] = {".config/", "."};
    unsigned int nprefixes = sizeof(prefixes) / sizeof(char *);
    for (unsigned int i = 0; i < nprefixes; i++)
    {
        int n = snprintf(config_path, sizeof(config_path), "%s/%s%src", home_dir, prefixes[i], PROGRAM_NAME);
        if (n < 0 || (unsigned int)n > sizeof(config_path) - 1)
            continue;

        if (!(config_fp = fopen(config_path, "r")))
            continue;

        break;
    }
    if (!config_fp)
        return -1;

    char *cmd_line = NULL;
    size_t capacity = 0;
    ssize_t line_len = 0;
    while ((line_len = getline(&cmd_line, &capacity, config_fp)) > 0)
        cmd_parse_and_execute_command_line(state, cmd_line);
    free(cmd_line);
    return 0;
}
