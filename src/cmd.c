#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <sys/errno.h>

#include "linenoise.h"

#include "argparse.h"
#include "cmd.h"
#include "prefix.h"
#include "state.h"

extern State state;
extern SeqTypeOption seq_type_options;
extern unsigned int n_seq_type_options;

PrefixTree cmd_map;

Command cmds[] = {
    {&cmd_quit, "quit"},
    {&cmd_next_file, "next"},
    {&cmd_previous_file, "previous"},
    {&cmd_set, "set"},
    {&cmd_type, "type"},
    {&cmd_scheme, "scheme"},
};

#define ncmds sizeof(cmds) / sizeof(Command)

// PARSING HELPERS
static int parse_integer(char *str, long *value)
{
    char *endptr;
    errno = 0;
    long tmp = strtol(str, &endptr, 10);
    if (errno == ERANGE || endptr == str || *endptr != '\0')
        return -1;
    *value = tmp;
    return 0;
}

static SeqColorScheme *parse_color_scheme_name(char *color_scheme_name)
{

    for (unsigned int i = 0; i < N_BASE_SCHEMES; i++)
    {
        SeqColorScheme *color_scheme = schemes_base + i;
        if (strcmp(color_scheme->scheme.name, color_scheme_name) == 0)
            return color_scheme;
    }
    for (unsigned int i = 0; i < state.n_color_schemes; i++)
    {
        SeqColorScheme *color_scheme = state.color_schemes + i;
        if (strcmp(color_scheme->scheme.name, color_scheme_name) == 0)
            return color_scheme;
    }
    return NULL;
}

// PARSING INTERFACE

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
        return -1;

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
        return -1;
    void *ptr = prefix_tree_get_prefix_match(&cmd_map, argv[0]);
    if (!ptr)
        return -1;
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
    if (retcode != 0)
        return retcode;
    for (unsigned int i = 0; i < ncmds; i++)
    {
        Command *cmd = cmds + i;
        retcode = prefix_tree_insert(&cmd_map, cmd->name, cmd);
        if (retcode != 0)
            return retcode;
    }
    return retcode;
}

void cmd_deinit_command_map(void)
{
    prefix_tree_deinit(&cmd_map);
}

// COMMANDS

/*
 * quit
 */
void cmd_quit(int argc, char **argv)
{
    (void)argc; // Silence warnings
    (void)argv;

    exit(EXIT_SUCCESS);
}

/*
 * next
 */
void cmd_next_file(int argc, char **argv)
{
    (void)argc; // Silence warnings
    (void)argv;

    if (state.active_file_index + 1 >= state.nfiles)
        return;
    state_set_active_file_index(&state, state.active_file_index + 1);
    state.refresh_window = true;
}

/*
 * previous
 */
void cmd_previous_file(int argc, char **argv)
{
    (void)argc; // Silence warnings
    (void)argv;

    if (state.active_file_index == 0)
        return;
    state_set_active_file_index(&state, state.active_file_index - 1);
    state.refresh_window = true;
}

/*
 * set <name> [<value>]
 */
void cmd_set(int argc, char **argv)
{
    if (argc == 3 && strcmp("ruler_records_divider", argv[1]) == 0)
    {
        // Parse <value> into unsigned int
        char *value_arg = argv[2];
        long value;
        int retcode = parse_integer(value_arg, &value);
        if (retcode != 0 || value < 0 || value > UINT_MAX)
            return;
        unsigned int ruler_records_divider = value;

        state_set_ruler_records_divider(&state, ruler_records_divider);
    }

    if (argc == 3 && strcmp("header_sequence_divider", argv[1]) == 0)
    {
        // Parse <value> into unsigned int
        char *value_arg = argv[2];
        long value;
        int retcode = parse_integer(value_arg, &value);
        if (retcode != 0 || value < 0 || value > UINT_MAX)
            return;
        unsigned int header_sequence_divider = value;

        state_set_header_sequence_divider(&state, header_sequence_divider);
    }

    if (argc == 3 && strcmp("tick_offset", argv[1]) == 0)
    {
        // Parse <value> into unsigned int
        char *value_arg = argv[2];
        long value;
        int retcode = parse_integer(value_arg, &value);
        if (retcode != 0 || value < 0 || value > UINT_MAX)
            return;
        unsigned int tick_offset = value;

        state_set_tick_offset(&state, tick_offset);
    }

    if (argc == 3 && strcmp("tick_spacing", argv[1]) == 0)
    {
        // Parse <value> into unsigned int
        char *value_arg = argv[2];
        long value;
        int retcode = parse_integer(value_arg, &value);
        if (retcode != 0 || value < 0 || value > UINT_MAX)
            return;
        unsigned int tick_spacing = value;

        state_set_tick_spacing(&state, tick_spacing);
    }

    if (argc == 3 && strcmp("scheme", argv[1]) == 0)
    {
        // Parse <value> into color scheme
        char *color_scheme_arg = argv[2];
        SeqColorScheme *color_scheme = parse_color_scheme_name(color_scheme_arg);
        if (!color_scheme)
            return;

        state_set_active_color_scheme(&state, color_scheme);
    }
}

/*
 * type <seq_type>
 */
void cmd_type(int argc, char **argv)
{
    if (argc != 2)
        return;

    // Parse <seq_type>
    char *seq_type_arg = argv[1];
    SeqType seq_type = argparse_seq_type(seq_type_arg, n_seq_type_options, &seq_type_options);
    if (seq_type == SEQ_TYPE_UNSPECIFIED)
        return;

    FileState *active_file = state.active_file;
    RowLinkedScroller *scroller = &active_file->layout.scroller;
    size_t record_index = scroller->offset_i + scroller->cursor_i;
    SeqRecord *record = active_file->record_array.data + record_index;
    record->type = seq_type;
    state.refresh_sequence_pane = true;
}

/*
 * scheme new <name> <seq_type>
 * scheme map <name> <sym> <value>
 */
void cmd_scheme(int argc, char **argv)
{
    if (argc > 1 && strcmp("new", argv[1]) == 0)
    {
        if (argc != 4)
            return;

        char *name = argv[2];

        // Parse <seq_type>
        char *seq_type_arg = argv[3];
        SeqType seq_type = argparse_seq_type(seq_type_arg, n_seq_type_options, &seq_type_options);
        if (seq_type == SEQ_TYPE_UNSPECIFIED)
            return;

        state_new_color_scheme(&state, name, seq_type, COLOR_8_BIT);
    }

    if (argc > 1 && strcmp("map", argv[1]) == 0)
    {
        if (argc != 5)
            return;

        // Parse <name> into color scheme
        char *color_scheme_arg = argv[2];
        SeqColorScheme *color_scheme = parse_color_scheme_name(color_scheme_arg);
        if (!color_scheme)
            return;

        // Parse <sym>
        char *sym_arg = argv[3];
        if (strlen(sym_arg) != 1)
            return;
        char sym = sym_arg[0];
        Alphabet *alphabet = sequences_seq_type_to_alphabet(color_scheme->type);
        if (!alphabet)
            return;
        int index = alphabet->index_map[(unsigned int)sym];
        if (index < 0)
            return;

        // Parse <value>
        char *value_arg = argv[4];
        long value;
        int retcode = parse_integer(value_arg, &value);
        if (retcode != 0 || value < 0 || value > UINT8_MAX)
            return;
        Color color;
        color.u8 = value;

        color_scheme_map_fg_color(&color_scheme->scheme, color, index);
    }
}
