#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <sys/errno.h>

#include "linenoise.h"

#include "cmd.h"
#include "state.h"
#include "prefix.h"

extern State state;

PrefixTree cmd_map;

Command cmds[] = {
    {&cmd_quit, "quit"},
    {&cmd_next_file, "next"},
    {&cmd_previous_file, "previous"},
    {&cmd_set, "set"},
};

#define NCMDS sizeof(cmds) / sizeof(Command)

char *cmd_read_command_line(int input_fd, char *prompt)
{
    struct linenoiseState ls;
    char buffer[1024];
    linenoiseEditStart(&ls, input_fd, -1, buffer, sizeof(buffer), prompt);
    char *line = NULL;
    while ((line = linenoiseEditFeed(&ls)) == linenoiseEditMore)
        ;
    linenoiseEditStop(&ls);
    return line;
}

int cmd_parse_and_execute_command_line(char *line)
{
    if (!line)
        return 1;

    int argc = 0;
    char *argv[CMD_ARG_MAX];

    int i = 0;
    for (int j = 0; line[j] != '\0'; j++)
    {
        if (isspace(line[j]))
            continue;

        // Start of word
        i = j;

        // End of word
        while (line[j] != '\0' && !isspace(line[j]))
            j++;

        // Extract word
        char *arg = strndup(line + i, j - i);
        if (!arg)
            goto cleanup;

        argv[argc++] = arg;

        if (argc == CMD_ARG_MAX)
            goto cleanup;
        if (line[j] == '\0')
            break;
    }

    if (argc == 0)
        return 1;
    void *ptr = prefix_tree_get_prefix_match(&cmd_map, argv[0]);
    if (!ptr)
        return 1;
    Command *cmd = ptr;

    void (*fn)(int, char **) = cmd->fn_ptr;
    fn(argc, argv);

cleanup:
    for (int i = 0; i < argc; i++)
        free(argv[i]);

    return 0;
}

int cmd_init_command_map(void)
{
    int retcode = prefix_tree_init(&cmd_map, CMD_ALPHABET, sizeof(Command));
    if (retcode > 0)
        return retcode;
    for (unsigned int i = 0; i < NCMDS; i++)
    {
        Command *cmd = cmds + i;
        retcode = prefix_tree_insert(&cmd_map, cmd->name, cmd);
        if (retcode > 0)
            return retcode;
    }
    return retcode;
}

void cmd_deinit_command_map(void)
{
    prefix_tree_deinit(&cmd_map);
}

void cmd_quit(int argc, char **argv)
{
    (void)argc; // Silence warnings
    (void)argv;

    exit(EXIT_SUCCESS);
}

void cmd_next_file(int argc, char **argv)
{
    (void)argc; // Silence warnings
    (void)argv;

    if (state.active_file_index + 1 >= state.nfiles)
        return;
    state_set_active_file_index(&state, state.active_file_index + 1);
    state.refresh_window = true;
}

void cmd_previous_file(int argc, char **argv)
{
    (void)argc; // Silence warnings
    (void)argv;

    if (state.active_file_index == 0)
        return;
    state_set_active_file_index(&state, state.active_file_index - 1);
    state.refresh_window = true;
}

void cmd_set(int argc, char **argv)
{
    if (argc == 3 && strcmp("tick_offset", argv[1]) == 0)
    {
        char *endptr;
        errno = 0;
        long value = strtol(argv[2], &endptr, 10);

        if (errno == ERANGE || endptr == argv[2] || *endptr != '\0')
            return;
        if (value < 0 || value > UINT_MAX)
            return;
        unsigned int tick_offset = value;

        state_set_tick_offset(&state, tick_offset);

        return;
    }

    if (argc == 3 && strcmp("type", argv[1]) == 0)
    {
        SeqType type;
        if (strcmp("nucleic", argv[2]) == 0)
            type = SEQ_TYPE_NUCLEIC;
        else if (strcmp("protein", argv[2]) == 0)
            type = SEQ_TYPE_PROTEIN;
        else
            return;

        FileState *active_file = state.active_file;
        RowLinkedScroller *scroller = &active_file->layout.scroller;
        size_t record_index = scroller->offset_i + scroller->cursor_i;
        SeqRecord *record = active_file->record_array.data + record_index;
        record->type = type;
        state.refresh_sequence_pane = true;

        return;
    }
}
