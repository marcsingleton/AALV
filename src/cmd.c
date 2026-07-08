#include <ctype.h>
#include <fnmatch.h>
#include <stdio.h>
#include <string.h>
#include <sys/errno.h>

#include "linenoise.h"

#include "argparse.h"
#include "cmd.h"
#include "config.h"
#include "io.h"
#include "prefix.h"

PrefixTree cmd_map;

Command cmds[] = {
    {&cmd_quit, "quit"},
    {&cmd_quit, "q"},
    {&cmd_quitall, "quitall"},
    {&cmd_quitall, "qall"},
    {&cmd_next_file, "next"},
    {&cmd_previous_file, "previous"},
    {&cmd_set, "set"},
    {&cmd_type, "type"},
    {&cmd_scheme, "scheme"},
    {&cmd_config, "config"},
    {&cmd_edit, "edit"},
};
unsigned int ncmds = sizeof(cmds) / sizeof(Command);

// Private
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

static SeqColorScheme *parse_color_scheme_name(State *state, char *color_scheme_name)
{
    for (unsigned int i = 0; i < N_BASE_SCHEMES; i++)
    {
        SeqColorScheme *color_scheme = schemes_base + i;
        if (strcmp(color_scheme->scheme.name, color_scheme_name) == 0)
            return color_scheme;
    }
    for (unsigned int i = 0; i < state->n_color_schemes; i++)
    {
        SeqColorScheme *color_scheme = state->color_schemes + i;
        if (strcmp(color_scheme->scheme.name, color_scheme_name) == 0)
            return color_scheme;
    }
    return NULL;
}

// Public
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

int cmd_parse_and_execute_command_line(State *state, char *line)
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

    void (*fn)(State *, int, char **) = cmd->fn_ptr;
    fn(state, argc, argv);

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

// Commands

/*
 * quit
 */
void cmd_quit(State *state, int argc, char **argv)
{
    (void)argc; // Silence warnings
    (void)argv;

    state_remove_file(state);
}

/*
 * quitall
 */

void cmd_quitall(State *state, int argc, char **argv)
{
    (void)argc; // Silence warnings
    (void)argv;

    // TODO: If files are editable, check none are modified before closing any to mimic Vim behavior
    while (state->nfiles > 0)
        state_remove_file(state);
}

/*
 * next
 */
void cmd_next_file(State *state, int argc, char **argv)
{
    (void)argc; // Silence warnings
    (void)argv;

    if (state->active_file_index + 1 >= state->nfiles)
        return;
    state_set_active_file_index(state, state->active_file_index + 1);
    state->refresh_window = true;
}

/*
 * previous
 */
void cmd_previous_file(State *state, int argc, char **argv)
{
    (void)argc; // Silence warnings
    (void)argv;

    if (state->active_file_index == 0)
        return;
    state_set_active_file_index(state, state->active_file_index - 1);
    state->refresh_window = true;
}

/*
 * set <name> [<value>]
 */
void cmd_set(State *state, int argc, char **argv)
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

        state_set_ruler_records_divider(state, ruler_records_divider);
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

        state_set_header_sequence_divider(state, header_sequence_divider);
    }

    if (argc == 3 && strcmp("tick_offset", argv[1]) == 0)
    {
        // Parse <value> into unsigned int
        char *value_arg = argv[2];
        long value;
        int retcode = parse_integer(value_arg, &value);
        if (retcode != 0 || value < INT_MIN || value > INT_MAX)
            return;
        int tick_offset = value;

        state_set_tick_offset(state, tick_offset);
    }

    if (argc == 3 && strcmp("tick_spacing", argv[1]) == 0)
    {
        // Parse <value> into unsigned int
        char *value_arg = argv[2];
        long value;
        int retcode = parse_integer(value_arg, &value);
        if (retcode != 0 || value < 0 || value > INT_MAX)
            return;
        int tick_spacing = value;

        state_set_tick_spacing(state, tick_spacing);
    }

    if (argc == 3 && strcmp("scheme", argv[1]) == 0)
    {
        // Parse <value> into color scheme
        char *color_scheme_arg = argv[2];
        SeqColorScheme *color_scheme = parse_color_scheme_name(state, color_scheme_arg);
        if (!color_scheme)
            return;

        state_set_active_color_scheme(state, color_scheme);
    }
}

/*
 * type <seq_type> [<id_pattern>]
 */
void cmd_type(State *state, int argc, char **argv)
{
    if (!(argc == 2 || argc == 3))
        return;

    // Parse <seq_type>
    char *seq_type_arg = argv[1];
    SeqType seq_type = argparse_seq_type(seq_type_arg, n_seq_type_options, seq_type_options);
    if (seq_type == SEQ_TYPE_UNSPECIFIED)
        return;

    // Apply to record(s)
    FileState *active_file = state->active_file;
    if (argc == 2)
    {
        RowLinkedScroller *scroller = &active_file->layout.scroller;
        size_t record_index = scroller->offset_i + scroller->cursor_i;
        SeqRecord *record = active_file->record_array.data + record_index;
        record->type = seq_type;
    }
    else if (argc == 3)
    {
        char *id_pattern = argv[2];
        int flags = 0;
        for (size_t record_index = 0; record_index < active_file->record_array.len; record_index++)
        {
            SeqRecord *record = active_file->record_array.data + record_index;
            if (fnmatch(id_pattern, record->id, flags) == 0)
                record->type = seq_type;
        }
    }
    state->refresh_sequence_pane = true;
}

/*
 * scheme new <name> <seq_type>
 * scheme map fg|bg <name> <sym> <value>
 */
void cmd_scheme(State *state, int argc, char **argv)
{
    if (argc > 1 && strcmp("new", argv[1]) == 0)
    {
        if (argc != 4)
            return;

        char *name = argv[2];

        // Parse <seq_type>
        char *seq_type_arg = argv[3];
        SeqType seq_type = argparse_seq_type(seq_type_arg, n_seq_type_options, seq_type_options);
        if (seq_type == SEQ_TYPE_UNSPECIFIED)
            return;

        state_new_color_scheme(state, name, seq_type, COLOR_8_BIT);
    }

    if (argc > 1 && strcmp("map", argv[1]) == 0)
    {
        if (argc != 6)
            return;

        // Parse fg/bg
        char *color_ground = argv[2];

        // Parse <name> into color scheme
        char *color_scheme_arg = argv[3];
        SeqColorScheme *color_scheme = parse_color_scheme_name(state, color_scheme_arg);
        if (!color_scheme)
            return;

        // Parse <sym>
        char *sym_arg = argv[4];
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
        char *value_arg = argv[5];
        long value;
        int retcode = parse_integer(value_arg, &value);
        if (retcode != 0 || value < 0 || value > UINT8_MAX)
            return;
        Color color;
        color.u8 = value;

        if (strcmp("fg", color_ground) == 0)
            color_scheme_map_fg_color(&color_scheme->scheme, color, index);
        else if (strcmp("bg", color_ground) == 0)
            color_scheme_map_bg_color(&color_scheme->scheme, color, index);
    }
}

/*
 * config <name> <value>
 */
void cmd_config(State *state, int argc, char **argv)
{
    (void)state; // Silence warnings

    if (argc == 3 && strcmp("ruler_records_divider", argv[1]) == 0)
    {
        // Parse <value> into unsigned int
        char *value_arg = argv[2];
        long value;
        int retcode = parse_integer(value_arg, &value);
        if (retcode != 0 || value < 0 || value > UINT_MAX)
            return;
        unsigned int ruler_records_divider = value;

        config_set_ruler_records_divider(ruler_records_divider);
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

        config_set_header_sequence_divider(header_sequence_divider);
    }

    if (argc == 3 && strcmp("tick_offset", argv[1]) == 0)
    {
        // Parse <value> into unsigned int
        char *value_arg = argv[2];
        long value;
        int retcode = parse_integer(value_arg, &value);
        if (retcode != 0 || value < INT_MIN || value > INT_MAX)
            return;
        int tick_offset = value;

        config_set_tick_offset(tick_offset);
    }

    if (argc == 3 && strcmp("tick_spacing", argv[1]) == 0)
    {
        // Parse <value> into unsigned int
        char *value_arg = argv[2];
        long value;
        int retcode = parse_integer(value_arg, &value);
        if (retcode != 0 || value < 0 || value > INT_MAX)
            return;
        int tick_spacing = value;

        config_set_tick_spacing(tick_spacing);
    }

    if (argc == 3 && strcmp("nucleic_tiebreak_len", argv[1]) == 0)
    {
        // Parse <value> into unsigned int
        char *value_arg = argv[2];
        long value;
        int retcode = parse_integer(value_arg, &value);
        if (retcode != 0 || value < 0 || value > UINT_MAX)
            return;
        unsigned int nucleic_tiebreak_len = value;

        config_set_nucleic_tiebreak_len(nucleic_tiebreak_len);
    }
}

/*
 * edit <file>
 */
void cmd_edit(State *state, int argc, char **argv)
{
    if (argc != 2)
        return;

    char *file_path = argv[1];

    // Close current and open new
    if (state_remove_file(state) != 0)
        return;
    if (state_new_file(state) != 0)
        return;
    if (state_set_file_path(state->active_file, file_path) != 0)
        return;

    // Get reader
    char *format_arg = strrchr(file_path, '.');
    if (format_arg)
        format_arg++; // Shift past dot
    else
        return;
    FileReader reader = argparse_reader(format_arg, n_format_options, format_options);

    // Open file
    FILE *fp = fopen(file_path, "r");
    if (!fp)
        return;

    // Load seqs and set metadata
    io_load_seqs(state->active_file, fp, reader);
    io_set_seq_types(state->active_file, "");
    io_set_unaligned_indices(state->active_file);
}
