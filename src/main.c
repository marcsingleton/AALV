#include <getopt.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>
#include <unistd.h>

#include "array.h"
#include "display.h"
#include "error.h"
#include "fasta.h"
#include "input.h"
#include "sequences.h"
#include "state.h"
#include "str.h"
#include "terminal.h"

#define PROGRAM_NAME "aalv"

State state;

struct termios old_termios;
struct termios raw_termios;
bool raw_mode = false;

extern char error_message[ERROR_MESSAGE_LEN];

typedef enum
{
    OMIT,
    SHORT_NAME,
    LONG_NAME,
    ALL_NAMES,
} UsageStyle;

typedef struct
{
    const char *long_name;
    const char short_name;
    const char *description;
    const char *usage_arg;
    const UsageStyle usage_style;
    const int has_arg; // getopt_long field
} Argument;

typedef struct
{
    const char *name;
    const char *exts;
    int (*reader)(FILE *, SeqRecord **);
} Format;

typedef struct
{
    const char *name;
    const char *identifiers;
} SeqType;

void cleanup(void);
int parse_options(int argc, char *argv[],
                  const char *short_options, const struct option *long_options,
                  char ***format_args_ptr, unsigned int *n_format_args,
                  char ***type_args_ptr, unsigned int *n_type_args);
int prepare_options(char **short_options, struct option *long_options);
int read_files(State *state, char **file_paths, char **format_args, unsigned int n_format_args);
void print_long_help(void);
void print_short_help(void);
int print_option_usage(Argument *argument, UsageStyle usage_style, const bool brackets, const char *style_sep);

// Order
Argument arguments[] = {
    {"help",
     'h',
     "print usage and options then exit",
     "",
     SHORT_NAME,
     no_argument},
    {"version",
     'v',
     "print version then exit",
     "",
     OMIT,
     no_argument},
    {"format",
     'f',
     "comma-separated list of format extensions for input files",
     "<fmt,...,fmt>",
     SHORT_NAME,
     required_argument},
    {"list-formats",
     0,
     "list allowable formats and their recognized extensions then exit",
     "",
     OMIT,
     no_argument},
    {"list-types",
     0,
     "list allowable types and their recognized identifiers then exit",
     "",
     OMIT,
     no_argument},
    {"type",
     't',
     "comma-separated list of sequence types for input files",
     "<type,...,type>",
     SHORT_NAME,
     required_argument},
};

#define NARGUMENTS sizeof(arguments) / sizeof(Argument)

Format formats[] = {
    {"FASTA", "fasta,fa,faa,fna,afa", &fasta_fread},
    // CLUSTAL
    // PHYLIP
    // STOCKHOLM
};

#define NFORMATS sizeof(formats) / sizeof(Format)

SeqType types[] = {
    {"nucleic", "nucleic,nt"},
    {"protein", "protein,aa"},
};

#define NTYPES sizeof(types) / sizeof(SeqType)

// Main
int main(int argc, char *argv[])
{
    int code = 0; // Generic return code for various functions
    atexit(&cleanup);

    // Prepare options
    struct option long_options[NARGUMENTS + 1]; // Extra struct of 0s to mark end
    char *short_options = NULL;
    code = prepare_options(&short_options, long_options);
    if (code != 0)
        return code;

    // Parse options
    char **format_args = NULL;
    unsigned int n_format_args = 0;
    char **type_args = NULL;
    unsigned int n_type_args = 0;
    code = parse_options(argc, argv,
                         short_options, long_options,
                         &format_args, &n_format_args,
                         &type_args, &n_type_args);
    free(short_options);
    if (code > 0) // "Expected" exit == 1 and "unexpected" exit > 1; shift -1 for CLI convention
        return code - 1;

    // Check for positional arguments
    if (isatty(STDIN_FILENO) && optind == argc)
    {
        print_short_help();
        return 1;
    }

    // Read files
    unsigned int nfiles = argc - optind;
    if (!isatty(STDIN_FILENO))
        nfiles++; // If not a tty, treat stdin as an implicit first file

    FileState *files = malloc(nfiles * sizeof(FileState));
    if (files == NULL)
    {
        strncpy(error_message, PROGRAM_NAME ": Failed to allocate memory to load files\n", ERROR_MESSAGE_LEN);
        return 1;
    }
    state.files = files;
    state.nfiles = nfiles;
    state.active_file = files;
    state.active_file_index = 0;

    code = read_files(&state, argv + optind, format_args, n_format_args);
    if (code > 0)
        return code - 1;

    if (n_format_args > 0)
        str_free_split(format_args, n_format_args);
    if (n_type_args > 0)
        str_free_split(type_args, n_type_args);

    // Set screen and terminal options
    if (terminal_get_termios(&old_termios) != 0)
    {
        strncpy(error_message, PROGRAM_NAME ": Failed to get current termios\n", ERROR_MESSAGE_LEN);
        return 1;
    }
    if (terminal_enable_raw_mode(&old_termios, &raw_termios) != 0)
    {
        strncpy(error_message, PROGRAM_NAME ": Failed to set raw mode", ERROR_MESSAGE_LEN);
        return 1;
    };
    raw_mode = true;
    terminal_use_alternate_buffer();

    setlocale(LC_ALL, ""); // Necessary for wcswidth calls

    // Main loop
    // Display current file
    // Read input
    // Process input
    int action;
    Array buffer;
    array_init(&buffer, sizeof(char));

    while (1)
    {
        action = input_get_action();
        input_process_action(action, &buffer);

        // Re-paint screen if necessary
        terminal_cursor_hide(&buffer);
        state.refresh_command_pane = true;

        unsigned int rows, cols;
        terminal_get_window_size(&rows, &cols);
        if (state.terminal_rows != rows || state.terminal_cols != cols)
        {
            state.terminal_rows = rows;
            state.terminal_cols = cols;
            state.refresh_window = true;
        }
        if (state.refresh_window)
        {
            terminal_clear_screen(&buffer);
            state_set_header_pane_width(&state, state.active_file->header_pane_width); // Triggers automatic re-sizes
            state_set_ruler_pane_height(&state, state.active_file->ruler_pane_height);
            state.refresh_ruler_pane = true;
            state.refresh_header_pane = true;
            state.refresh_sequence_pane = true;
            state.refresh_command_pane = true;
            state.refresh_window = false;
        }
        if (state.refresh_ruler_pane)
        {
            display_ruler_pane(&buffer);
            display_ruler_pane_ticks(&buffer);
            state.refresh_ruler_pane = false;
        }
        if (state.refresh_header_pane)
        {
            display_header_pane(&buffer);
            state.refresh_header_pane = false;
        }
        if (state.refresh_sequence_pane)
        {
            display_sequence_pane(&buffer);
            state.refresh_sequence_pane = false;
        }
        if (state.refresh_command_pane)
        {
            display_command_pane(&buffer);
            state.refresh_command_pane = false;
        }
        display_cursor(&buffer);

        input_buffer_flush(&buffer);
    }
}

void cleanup(void)
{
    // Free memory
    for (unsigned int i = 0; i < state.nfiles; i++)
        sequences_free_seq_record_array(&state.files[i].record_array); // Null if unset, so always safe to free
    free(state.files);

    // Restore terminal options
    if (raw_mode)
    {
        terminal_use_normal_buffer();
        terminal_disable_raw_mode(&old_termios);
    }

    // Print error
    if (error_message[0] != '\0')
        fputs(error_message, stderr);
}

// Argument parsing
int parse_options(int argc, char *argv[],
                  const char *short_options, const struct option *long_options,
                  char ***format_args_ptr, unsigned int *n_format_args,
                  char ***type_args_ptr, unsigned int *n_type_args)
{
    while (1)
    {
        // Check if next argument is a known option
        int option_index = -1;
        int c = getopt_long(argc, argv, short_options, long_options, &option_index);
        if (c == -1)
            break;
        else if (c == '?')
        {
            print_short_help();
            return 2;
        }

        // Process option
        const char *name = "";
        if (option_index != -1)
            name = arguments[option_index].long_name;
        if (c == 'f' || strcmp(name, "format") == 0)
        {
            int code = str_split(format_args_ptr, argv[optind - 1], ',');
            if (code < 0)
            {
                strncpy(error_message, PROGRAM_NAME ": Failed to parse formats\n", ERROR_MESSAGE_LEN);
                return 2;
            }
            *n_format_args = code;
            printf("Identified the following formats:\n");
            for (unsigned int i = 0; i < *n_format_args; i++)
                printf("    %s\n", (*format_args_ptr)[i]);
        }
        else if (c == 'h')
        {
            print_short_help();
            return 1;
        }
        else if (strcmp(name, "help") == 0)
        {
            print_long_help();
            return 1;
        }
        else if (strcmp(name, "list-formats") == 0)
        {
            printf("Format\tExtensions\n");
            for (unsigned int i = 0; i < NFORMATS; i++)
            {
                Format *format = formats + i;
                printf("%s\t%s\n", format->name, format->exts);
            }
            return 1;
        }
        else if (strcmp(name, "list-types") == 0)
        {
            printf("Type\tIdentifiers\n");
            for (unsigned int i = 0; i < NTYPES; i++)
            {
                SeqType *type = types + i;
                printf("%s\t%s\n", type->name, type->identifiers);
            }
            return 1;
        }
        else if (c == 't' || strcmp(name, "type") == 0)
        {
            int code = str_split(type_args_ptr, argv[optind - 1], ',');
            if (code < 0)
            {
                strncpy(error_message, PROGRAM_NAME ": Failed to parse types\n", ERROR_MESSAGE_LEN);
                return 2;
            }
            *n_type_args = code;
            printf("Identified the following types:\n");
            for (unsigned int i = 0; i < *n_type_args; i++)
                printf("    %s\n", (*type_args_ptr)[i]);
        }
        else if (c == 'v' || strcmp(name, "version") == 0)
        {
            printf("aalv 0.1.0 dev\n");
            return 1;
        }
    }
    return 0;
}

int prepare_options(char **short_options_ptr, struct option *long_options)
{
    int code = 0;
    Array short_options_array;
    array_init(&short_options_array, sizeof(char));
    for (unsigned int i = 0; i < NARGUMENTS; i++)
    {
        Argument *argument = arguments + i;

        // Fill struct for long options
        struct option *long_option = long_options + i;
        long_option->name = argument->long_name;
        long_option->has_arg = argument->has_arg;
        long_option->flag = 0;
        long_option->val = 0;

        // Fill array for short options syntax
        if (!argument->short_name)
            continue;
        char s = argument->short_name;
        array_append(&short_options_array, &s);
        switch (argument->has_arg)
        {
        case required_argument:
        {
            char s[] = ":";
            array_extend(&short_options_array, &s, sizeof(s) - 1);
            break;
        }
        case optional_argument:
        {
            char s[] = "::";
            array_extend(&short_options_array, &s, sizeof(s) - 1);
            break;
        }
        }
    }
    char s[] = "\0"; // Null terminate
    array_extend(&short_options_array, &s, sizeof(s) - 1);

    // Convert array of short options to string
    char *short_options = malloc(short_options_array.len * short_options_array.size);
    if (short_options == NULL)
    {
        code = 1;
        strncpy(error_message, PROGRAM_NAME ": Failed to allocate memory to create options string\n", ERROR_MESSAGE_LEN);
        goto cleanup;
    }
    memcpy(short_options, short_options_array.data, short_options_array.len * short_options_array.size);
    *short_options_ptr = short_options;
cleanup:
    array_free(&short_options_array);
    return code;
}

int read_files(State *state, char **file_paths, char **format_args, unsigned int n_format_args)
{
    StrArray *formats_exts = malloc(NFORMATS * sizeof(StrArray));
    if (formats_exts == NULL)
    {
        strncpy(error_message, PROGRAM_NAME ": Failed to allocate memory to split format extensions\n", ERROR_MESSAGE_LEN);
        return 1;
    }
    for (unsigned int i = 0; i < NFORMATS; i++)
    {
        Format *format = formats + i;
        StrArray *format_exts = formats_exts + i;
        format_exts->len = str_split((char ***)&format_exts->data, format->exts, ',');
    }

    for (unsigned int file_index = 0; file_index < state->nfiles; file_index++)
    {
        // Infer reader from extension
        const char *file_path, *file_ext;
        file_path = file_paths[file_index];

        int (*reader)(FILE *, SeqRecord **) = NULL;
        if (file_index < n_format_args)
        {
            char *format_arg = format_args[file_index];
            for (unsigned int i = 0; i < NFORMATS; i++)
            {
                Format *format = formats + i;
                StrArray *format_exts = formats_exts + i;
                char **exts = (char **)format_exts->data;
                for (unsigned int j = 0; j < format_exts->len; j++)
                {
                    if (strcmp(format_arg, exts[j]) == 0)
                    {
                        reader = format->reader;
                        break;
                    }
                }
                if (reader != NULL)
                    break;
                snprintf(error_message, ERROR_MESSAGE_LEN, PROGRAM_NAME ": %s: Error identifying format\n", format_arg);
                return 2;
            }
        }
        else if ((file_ext = strrchr(file_path, '.')) != NULL)
        {
            file_ext++; // Exclude dot from comparison
            for (unsigned int i = 0; i < NFORMATS; i++)
            {
                Format *format = formats + i;
                StrArray *format_exts = formats_exts + i;
                char **exts = (char **)format_exts->data;
                for (unsigned int j = 0; j < format_exts->len; j++)
                {
                    if (strcmp(file_ext, exts[j]) == 0)
                    {
                        reader = format->reader;
                        break;
                    }
                }
                if (reader != NULL)
                    break;
                snprintf(error_message, ERROR_MESSAGE_LEN, PROGRAM_NAME ": %s: Unknown extension\n", file_path);
                return 2;
            }
        }
        else
        {
            snprintf(error_message, ERROR_MESSAGE_LEN, PROGRAM_NAME ": %s: No format or known extension\n", file_path);
            return 2;
        }

        // Read file
        FILE *fp;
        if (strcmp(file_path, "-") == 0)
            fp = stdin;
        else if ((fp = fopen(file_path, "r")) == NULL)
        {
            snprintf(error_message, ERROR_MESSAGE_LEN, PROGRAM_NAME ": %s: %s\n", file_path, strerror(errno));
            return 1;
        }
        SeqRecord *records = NULL;
        int len = reader(fp, &records);
        if (len < 0)
        {
            snprintf(error_message, ERROR_MESSAGE_LEN, PROGRAM_NAME ": %s: Error processing file (code %d)\n", file_path, len);
            return 1;
        }

        // Set sequence type
        // - Types: protein,nucleic,mixed,unknown
        // - For each sequence
        //      - Get possible sequence types
        //      - If types is given, extract type
        //          - If given type is compatible with inferred type, use given type
        //          - Otherwise, use inferred type

        FileState *file = state->files + file_index;
        file->record_array.records = records;
        file->record_array.len = len;
        file->record_array.offset = 1;
        file->header_pane_width = 30;
        file->ruler_pane_height = 5;
        file->tick_spacing = 10;
    }

    return 0;
}

// CLI output
void print_long_help(void)
{
    unsigned int nmax;
    unsigned int nchars;

    // Usage section
    char *prefix = "usage: " PROGRAM_NAME;
    int prefix_size = (int)(sizeof(prefix) - 1);

    nmax = 80;
    nchars = 0;

    nchars += printf("%s", prefix);
    for (unsigned int i = 0; i < NARGUMENTS; i++)
    {
        Argument *argument = arguments + i;
        UsageStyle usage_style = OMIT;
        if (argument->long_name && argument->short_name)
            usage_style = ALL_NAMES;
        else if (argument->long_name)
            usage_style = LONG_NAME;
        else if (argument->short_name)
            usage_style = SHORT_NAME;
        if (nchars > nmax)
        {
            putchar('\n');
            nchars = printf("%*s", prefix_size, "");
        }
        else
        {
            putchar(' ');
            nchars++;
        }
        nchars += print_option_usage(argument, usage_style, true, " |");
    }
    printf("\n%*s[<file> ...]\n\n", prefix_size, "");

    // Synopsis section
    printf("aalv is a vim-inspired alignment viewer\n\n");

    // Options section
    printf("options:\n");

    nmax = 24;
    for (unsigned int i = 0; i < NARGUMENTS; i++)
    {
        Argument *argument = arguments + i;
        UsageStyle usage_style = OMIT;
        if (argument->long_name && argument->short_name)
            usage_style = ALL_NAMES;
        else if (argument->long_name)
            usage_style = LONG_NAME;
        else if (argument->short_name)
            usage_style = SHORT_NAME;
        nchars = printf("    ");
        nchars += print_option_usage(argument, usage_style, false, ",");
        if (nchars > nmax - 2) // Two spaces of buffer
            printf("\n%*s", nmax, "");
        else
            printf("%*s", nmax - nchars, "");
        printf("%s\n", argument->description);
    }
}

void print_short_help(void)
{
    char *prefix = "usage: " PROGRAM_NAME;

    printf("%s", prefix);
    for (unsigned int i = 0; i < NARGUMENTS; i++)
    {
        Argument *argument = arguments + i;
        if (argument->usage_style == OMIT)
            continue;
        putchar(' ');
        print_option_usage(argument, argument->usage_style, true, " |");
    }
    printf(" [<file> ...]\n");
}

int print_option_usage(Argument *argument, UsageStyle usage_style, const bool brackets, const char *style_sep)
{
    const char *arg_sep = "";
    const char *arg = "";
    if (argument->has_arg)
    {
        arg_sep = " ";
        arg = argument->usage_arg;
    }
    const char *lbracket = "";
    const char *rbracket = "";
    if (brackets)
    {
        lbracket = "[";
        rbracket = "]";
    }
    switch (usage_style) // This can be poorly formatted if the usage_style does not match the arguments
    {
    case OMIT:
        return 0;
    case SHORT_NAME:
        return printf("%s"
                      "-%c"
                      "%s%s%s",
                      lbracket,
                      argument->short_name, arg_sep, arg,
                      rbracket);
    case LONG_NAME:
        return printf("%s"
                      "--%s%s%s"
                      "%s",
                      lbracket,
                      argument->long_name, arg_sep, arg,
                      rbracket);
    case ALL_NAMES:
        return printf("%s"
                      "--%s%s%s"
                      "%s "
                      "-%c%s%s"
                      "%s",
                      lbracket,
                      argument->long_name, arg_sep, arg,
                      style_sep,
                      argument->short_name, arg_sep, arg,
                      rbracket);
    }
}
