#include <ctype.h>
#include <fnmatch.h>
#include <stdio.h>
#include <string.h>
#include <sys/errno.h>

#include "linenoise.h"

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
    {&cmd_write, "write"},
};
unsigned int ncmds = sizeof(cmds) / sizeof(Command);

// Private
static int parse_integer(char *str, long long min, long long max, long long *value)
{
    char *endptr;
    errno = 0;
    long long tmp = strtoll(str, &endptr, 10);
    if (endptr == str || *endptr != '\0')
        return CMD_EINTPARSE;
    if (errno == ERANGE || tmp < min || tmp > max)
        return CMD_ERANGE;
    *value = tmp;
    return CMD_ESUCCESS;
}

static SeqColorScheme *parse_color_scheme_name(State *state, char *color_scheme_name)
{
    for (unsigned int i = 0; i < n_base_schemes; i++)
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
char *cmd_strerror(int error)
{
    switch ((CommandError)error)
    {
    case CMD_ESUCCESS:
        return "Success";
    case CMD_EARGNUM:
        return "Incorrect number of arguments";
    case CMD_EINVAL:
        return "Invalid argument";
    case CMD_EINTPARSE:
        return "Argument not valid integer";
    case CMD_ERANGE:
        return "Value out of range";
    case CMD_ENOMEM:
        return "Memory allocation failed";
    case CMD_EARGMAX:
        return "Exceeded maximum number of arguments";
    case CMD_EUNKCMD:
        return "Unknown command";
    case CMD_EUNKARG:
        return "Unknown option or subcommand";
    case CMD_ESEQTYPE:
        return "Unknown sequence type";
    case CMD_ESCHEME:
        return "Unknown color scheme";
    case CMD_EFORMAT:
        return "Unknown file format";
    case CMD_EOPEN:
        return "Failed to open file";
    case CMD_ENOPATH:
        return "No file path available";
    case CMD_EFILEOP:
        return "Failure in internal file operation";
    }
    // No default case to trigger warning for missed values

    return "Unknown error";
}

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

int cmd_parse_command_line(char *line, int *argc, char *argv[CMD_ARG_MAX])
{
    if (!line || !argc || !argv)
        return CMD_EINVAL;

    int retcode = CMD_ESUCCESS;
    int new_argc = 0;
    unsigned int j = 0;
    while (line[j] != '\0')
    {
        if (isspace(line[j]))
        {
            j++;
            continue;
        }

        // Check capacity
        if (new_argc == CMD_ARG_MAX)
        {
            retcode = CMD_EARGMAX;
            goto cleanup;
        }

        // Start of word
        unsigned int i = j;

        // End of word
        while (line[j] != '\0' && !isspace(line[j]))
            j++;

        // Extract word
        char *arg = strndup(line + i, j - i);
        if (!arg)
        {
            retcode = CMD_ENOMEM;
            goto cleanup;
        }
        argv[new_argc++] = arg;
    }
    *argc = new_argc;

    return retcode;

cleanup:
    for (int i = 0; i < new_argc; i++)
        free(argv[i]);

    return retcode;
}

int cmd_execute_command(State *state, int argc, char *argv[CMD_ARG_MAX])
{
    if (!state || argc <= 0 || !argv)
        return CMD_EINVAL;
    for (int i = 0; i < argc; i++)
        if (!argv[i])
            return CMD_EINVAL;

    void *ptr = prefix_tree_get_prefix_match(&cmd_map, argv[0]);
    if (!ptr)
        return CMD_EUNKCMD;
    Command *cmd = ptr;

    int (*fn)(State *, int, char **) = cmd->fn_ptr;

    return fn(state, argc, argv);
}

int cmd_parse_execute_command_line(State *state, char *line)
{
    if (!state || !line)
        return CMD_EINVAL;

    int retcode = CMD_ESUCCESS;

    int argc = 0;
    char *argv[CMD_ARG_MAX];

    retcode = cmd_parse_command_line(line, &argc, argv);
    if (retcode != CMD_ESUCCESS)
        goto cleanup;

    retcode = cmd_execute_command(state, argc, argv);

cleanup:
    for (int i = 0; i < argc; i++)
        free(argv[i]);

    return retcode;
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
int cmd_quit(State *state, int argc, char **argv)
{
    (void)argc; // Silence warnings
    (void)argv;

    if (state_remove_file(state) != 0)
        return CMD_EFILEOP;

    return CMD_ESUCCESS;
}

/*
 * quitall
 */

int cmd_quitall(State *state, int argc, char **argv)
{
    (void)argc; // Silence warnings
    (void)argv;

    // TODO: If files are editable, check none are modified before closing any to mimic Vim behavior
    while (state->nfiles > 0)
        if (state_remove_file(state) != 0)
            return CMD_EFILEOP;

    return CMD_ESUCCESS;
}

/*
 * next
 */
int cmd_next_file(State *state, int argc, char **argv)
{
    (void)argc; // Silence warnings
    (void)argv;

    if (state->active_file_index + 1 >= state->nfiles)
        return CMD_ESUCCESS;
    state_set_active_file_index(state, state->active_file_index + 1);
    state->refresh_window = true;

    return CMD_ESUCCESS;
}

/*
 * previous
 */
int cmd_previous_file(State *state, int argc, char **argv)
{
    (void)argc; // Silence warnings
    (void)argv;

    if (state->active_file_index == 0)
        return CMD_ESUCCESS;
    state_set_active_file_index(state, state->active_file_index - 1);
    state->refresh_window = true;

    return CMD_ESUCCESS;
}

/*
 * set <name> <value>
 */
int cmd_set(State *state, int argc, char **argv)
{
    if (argc != 3)
        return CMD_EARGNUM;

    char *name = argv[1];
    char *value_arg = argv[2];

    long long value;
    int retcode;

    if (strcmp("ruler_records_divider", name) == 0)
    {
        retcode = parse_integer(value_arg, 0, UINT_MAX, &value);
        if (retcode != CMD_ESUCCESS)
            return retcode;
        state_set_ruler_records_divider(state, (unsigned int)value);
    }
    else if (strcmp("header_sequence_divider", name) == 0)
    {
        retcode = parse_integer(value_arg, 0, UINT_MAX, &value);
        if (retcode != CMD_ESUCCESS)
            return retcode;
        state_set_header_sequence_divider(state, (unsigned int)value);
    }
    else if (strcmp("tick_offset", name) == 0)
    {
        retcode = parse_integer(value_arg, INT_MIN, INT_MAX, &value);
        if (retcode != CMD_ESUCCESS)
            return retcode;
        state_set_tick_offset(state, (int)value);
    }
    else if (strcmp("tick_spacing", name) == 0)
    {
        retcode = parse_integer(value_arg, 0, INT_MAX, &value);
        if (retcode != CMD_ESUCCESS)
            return retcode;
        state_set_tick_spacing(state, (int)value);
    }
    else if (strcmp("scheme", name) == 0)
    {
        SeqColorScheme *color_scheme = parse_color_scheme_name(state, value_arg);
        if (!color_scheme)
            return CMD_ESCHEME;
        state_set_active_color_scheme(state, color_scheme);
    }
    else
        return CMD_EUNKARG;

    return CMD_ESUCCESS;
}

/*
 * type <seq_type> [<id_pattern>]
 */
int cmd_type(State *state, int argc, char **argv)
{
    if (!(argc == 2 || argc == 3))
        return CMD_EARGNUM;

    // Parse <seq_type>
    char *seq_type_arg = argv[1];
    SeqType seq_type = io_get_seq_type(seq_type_arg, n_seq_type_options, seq_type_options);
    if (seq_type == SEQ_TYPE_UNSPECIFIED)
        return CMD_ESEQTYPE;

    // Apply to record(s)
    FileState *active_file = state->active_file;
    if (active_file->record_array.len == 0) // Necessary when argc == 2
        return CMD_ESUCCESS;

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

    return CMD_ESUCCESS;
}

/*
 * scheme new <name> <seq_type>
 * scheme remove <name>
 * scheme map fg|bg <name> [<sym>=<value>]
 */
int cmd_scheme(State *state, int argc, char **argv)
{
    if (argc < 2)
        return CMD_EARGNUM;

    if (strcmp("new", argv[1]) == 0)
    {
        if (argc != 4)
            return CMD_EARGNUM;

        char *name = argv[2];

        // Parse <seq_type>
        char *seq_type_arg = argv[3];
        SeqType seq_type = io_get_seq_type(seq_type_arg, n_seq_type_options, seq_type_options);
        if (seq_type == SEQ_TYPE_UNSPECIFIED)
            return CMD_ESEQTYPE;

        state_new_color_scheme(state, name, seq_type, COLOR_8_BIT);
    }
    else if (strcmp("remove", argv[1]) == 0)
    {
        if (argc != 3)
            return CMD_EARGNUM;

        char *name = argv[2];

        state_remove_color_scheme(state, name);
    }
    else if (strcmp("map", argv[1]) == 0)
    {
        if (argc < 4)
            return CMD_EARGNUM;

        // Parse fg/bg
        char *color_ground = argv[2];
        int is_fg = strcmp("fg", color_ground) == 0;
        int is_bg = strcmp("bg", color_ground) == 0;
        if (!is_fg && !is_bg)
            return CMD_EUNKARG;

        // Parse <name> into color scheme
        char *color_scheme_arg = argv[3];
        SeqColorScheme *color_scheme = parse_color_scheme_name(state, color_scheme_arg);
        if (!color_scheme)
            return CMD_ESCHEME;

        // Get corresponding alphabet
        Alphabet *alphabet = sequences_seq_type_to_alphabet(color_scheme->type);
        if (!alphabet)
            return CMD_ESEQTYPE;

        for (int i = 4; i < argc; i++)
        {
            char *arg = argv[i];
            char sym;
            int value;
            if (sscanf(arg, "%c=%d", &sym, &value) != 2)
                return CMD_EINVAL;

            // Parse <sym>
            int index = alphabet->index_map[(unsigned char)sym];
            if (index < 0)
                return CMD_EINVAL;

            // Parse <value>
            if (value < 0 || value > UINT8_MAX)
                return CMD_ERANGE;
            Color color;
            color.u8 = value;

            if (is_fg)
                color_scheme_map_fg_color(&color_scheme->scheme, color, index);
            else if (is_bg)
                color_scheme_map_bg_color(&color_scheme->scheme, color, index);
        }
    }
    else
        return CMD_EUNKARG;

    return CMD_ESUCCESS;
}

/*
 * config <name> <value>
 */
int cmd_config(State *state, int argc, char **argv)
{
    if (argc != 3)
        return CMD_EARGNUM;

    char *name = argv[1];
    char *value_arg = argv[2];

    long long value;
    int retcode;

    if (strcmp("ruler_records_divider", name) == 0)
    {
        retcode = parse_integer(value_arg, 0, UINT_MAX, &value);
        if (retcode != CMD_ESUCCESS)
            return retcode;
        config_set_ruler_records_divider(&state->config, (unsigned int)value);
    }
    else if (strcmp("header_sequence_divider", name) == 0)
    {
        retcode = parse_integer(value_arg, 0, UINT_MAX, &value);
        if (retcode != CMD_ESUCCESS)
            return retcode;
        config_set_header_sequence_divider(&state->config, (unsigned int)value);
    }
    else if (strcmp("tick_offset", name) == 0)
    {
        retcode = parse_integer(value_arg, INT_MIN, INT_MAX, &value);
        if (retcode != CMD_ESUCCESS)
            return retcode;
        config_set_tick_offset(&state->config, (int)value);
    }
    else if (strcmp("tick_spacing", name) == 0)
    {
        retcode = parse_integer(value_arg, 0, INT_MAX, &value);
        if (retcode != CMD_ESUCCESS)
            return retcode;
        config_set_tick_spacing(&state->config, (int)value);
    }
    else if (strcmp("nucleic_tiebreak_len", name) == 0)
    {
        retcode = parse_integer(value_arg, 0, UINT_MAX, &value);
        if (retcode != CMD_ESUCCESS)
            return retcode;
        config_set_nucleic_tiebreak_len(&state->config, (unsigned int)value);
    }
    else
        return CMD_EUNKARG;

    return CMD_ESUCCESS;
}

/*
 * edit <file>
 */
int cmd_edit(State *state, int argc, char **argv)
{
    if (argc != 2)
        return CMD_EARGNUM;

    char *file_path = argv[1];

    // Close current and open new
    if (state_remove_file(state) != 0 ||
        state_new_file(state) != 0 ||
        state_set_file_path(state->active_file, file_path) != 0)
        return CMD_EFILEOP;

    // Get reader
    const char *format_arg = io_get_path_extension(file_path);
    if (!format_arg)
        return CMD_EFORMAT;
    FileReader reader = io_get_reader(format_arg, n_format_options, format_options);
    if (!reader)
        return CMD_EFORMAT;

    // Open file
    FILE *fp = fopen(file_path, "r");
    if (!fp)
        return CMD_EOPEN;

    // Load seqs and set metadata
    io_load_seqs(state->active_file, fp, reader);
    io_set_seq_types(state->active_file, "", state->config.nucleic_tiebreak_len);
    io_set_unaligned_indices(state->active_file);

    // Close file
    fclose(fp);

    return CMD_ESUCCESS;
}

/*
 * write [<file> [<kw>=<arg> ...]]
 */
int cmd_write(State *state, int argc, char **argv)
{
    // Get file path
    char *file_path;
    if (argc > 1)
        file_path = argv[1];
    else if (state->active_file->file_path)
        file_path = state->active_file->file_path;
    else
        return CMD_ENOPATH;

    // Get writer
    const char *format_arg = io_get_path_extension(file_path);
    if (!format_arg)
        return CMD_EFORMAT;
    FileWriter writer = io_get_writer(format_arg, n_format_options, format_options);
    if (!writer)
        return CMD_EFORMAT;

    // Open file
    FILE *fp = fopen(file_path, "w");
    if (!fp)
        return CMD_EOPEN;

    // Get kwarg
    int kwargc = 0;
    char **kwargv = NULL;
    if (argc > 2)
    {
        kwargc = argc - 2;
        kwargv = argv + 2;
    }

    writer(fp, &state->active_file->record_array, kwargc, kwargv);

    // Close file
    fclose(fp);

    return CMD_ESUCCESS;
}
