#include <getopt.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "array.h"
#include "display.h"
#include "error.h"
#include "fasta.h"
#include "input.h"
#include "sequences.h"
#include "state.h"
#include "terminal.h"

#define PROGRAM_NAME "aalv"

State state;
extern char error_message[ERROR_MESSAGE_LEN];

void cleanup(void);
void print_usage(void);

typedef struct
{
    const char *long_name;
    const char short_name;
    const char *description;
    const char *usage;
    int has_arg;
} Argument;

typedef struct
{
    char *name;
    char *exts;
    int (*reader)(const char *, SeqRecord **);
} Format;

typedef struct
{
    char *name;
    char *identifiers;
} SeqType;

// Order
Argument arguments[] = {
    {"help",
     'h',
     "print usage and options then exit",
     "-h | --help",
     no_argument},
    {"version",
     'v',
     "print version then exit",
     "-v | --version",
     no_argument},
    {"format",
     'f',
     "comma-separated list of format extensions for input files",
     "-f <fmt,...,fmt>",
     required_argument},
    {"list-formats",
     0,
     "list allowable formats and their recognized extensions then exit",
     "--list-formats",
     no_argument},
    {"list-types",
     0,
     "list allowable types and their recognized identifiers then exit",
     "--list-types",
     no_argument},
    {"type",
     't',
     "comma-separated list of sequence types for input files",
     "-t <type,...,type>",
     required_argument},
};

#define NARGUMENTS sizeof(arguments) / sizeof(Argument)

Format formats[] = {
    {"FASTA", "fasta,fa,faa,fna,afa", &fasta_read},
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
    // Prepare arguments
    struct option long_options[NARGUMENTS + 1]; // Extra struct of 0s to mark end
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
        strncpy(error_message, PROGRAM_NAME ": Failed to allocate memory to create options string\n", ERROR_MESSAGE_LEN);
        return 1;
    }
    memcpy(short_options, short_options_array.data, short_options_array.len * short_options_array.size);
    array_free(&short_options_array);

    // Parse arguments
    char *format_args = NULL;
    char *type_args = NULL;
    while (1)
    {
        // Check if next argument is a known option
        int option_index = -1;
        int c = getopt_long(argc, argv, short_options, long_options, &option_index);
        if (c == -1)
            break;
        else if (c == '?')
        {
            print_usage();
            return 1;
        }

        // Process option
        const char *name = "";
        if (option_index != -1)
            name = arguments[option_index].long_name;
        if (c == 'f' || strcmp(name, "format") == 0)
        {
            format_args = argv[optind - 1];
            printf("Identified the following formats: %s\n", format_args);
        }
        else if (c == 'h')
        {
            print_usage();
            return 0;
        }
        else if (strcmp(name, "help") == 0)
        {
            printf("This is the long help message\n");
            return 0;
        }
        else if (strcmp(name, "list-formats") == 0)
        {
            printf("Format\tExtensions\n");
            for (unsigned int i = 0; i < NFORMATS; i++)
            {
                Format *format = formats + i;
                printf("%s\t%s\n", format->name, format->exts);
            }
            return 0;
        }
        else if (strcmp(name, "list-types") == 0)
        {
            printf("Type\tIdentifiers\n");
            for (unsigned int i = 0; i < NTYPES; i++)
            {
                SeqType *type = types + i;
                printf("%s\t%s\n", type->name, type->identifiers);
            }
            return 0;
        }
        else if (c == 't' || strcmp(name, "type") == 0)
        {
            type_args = argv[optind - 1];
            printf("Identified the following types: %s\n", type_args);
        }
        else if (c == 'v' || strcmp(name, "version") == 0)
        {
            printf("aalv 0.1.0 dev\n");
            return 0;
        }
    }

    free(short_options);

    // Check for positional arguments
    if (isatty(STDIN_FILENO) && optind == argc)
    {
        print_usage();
        return 1;
    }

    // Set screen and terminal options
    if (terminal_get_termios(&state.old_termios) != 0)
    {
        strncpy(error_message, PROGRAM_NAME ": Failed to get current termios\n", ERROR_MESSAGE_LEN);
        return 1;
    }
    atexit(&cleanup); // Only register when get_termios is successful
    if (terminal_enable_raw_mode(&state.old_termios, &state.raw_termios) != 0)
    {
        strncpy(error_message, PROGRAM_NAME ": Failed to set raw mode", ERROR_MESSAGE_LEN);
        return 1;
    };
    terminal_use_alternate_buffer();

    setlocale(LC_ALL, ""); // Necessary for wcswidth calls

    // Read files
    FileState *files = malloc((argc - 1) * sizeof(FileState));
    if (files == NULL)
    {
        strncpy(error_message, PROGRAM_NAME ": Failed to allocate memory to load files\n", ERROR_MESSAGE_LEN);
        return 1;
    }
    state.files = files;
    state.nfiles = argc - 1;
    state.active_file = files;
    state.active_file_index = 0;

    for (unsigned int i = 0; i < state.nfiles; i++)
    {
        // Iterate over provided arguments following options
        // - File path is "loop" variable
        // - Special case: If a tty, consider - as implicit first path
        //      - malloc call assign extra slot as well

        // Parser selection logic: takes the input arguments and selects a parser
        // - Attempt to extract format from formats
        //      - Exit if no match
        // - Attempt to extract format from path
        //      - Special case: - (stdin) needs a format
        //      - If no match, infer with sniffer
        //          - Exit if no sniffer is successful
        // - Exit if an explicit or inferred format is malformed

        // Parsing logic
        // - Open file from path
        //      - Special case: - provide stdin
        // - Call parser

        char *file_path = argv[optind + i];
        SeqRecord *records = NULL;
        int len = fasta_read(file_path, &records);
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

        FileState *file = state.files + i;
        file->record_array.records = records;
        file->record_array.records = records;
        file->record_array.len = len;
        file->record_array.offset = 990;
        file->header_pane_width = 30;
        file->ruler_pane_height = 5;
        file->tick_spacing = 10;
    }

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
    terminal_use_normal_buffer();
    terminal_disable_raw_mode(&state.old_termios);

    // Print error
    if (error_message[0] != '\0')
        fputs(error_message, stderr);
}

void print_usage(void)
{
    printf("usage: " PROGRAM_NAME " [<file> ...]\n");
}
