#include <fcntl.h>
#include <getopt.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>
#include <unistd.h>

#include <curses.h>
#include <term.h> // Must follow curses

#include "argparse.h"
#include "array.h"
#include "display.h"
#include "error.h"
#include "fasta.h"
#include "input.h"
#include "rcparams.h"
#include "schemes.h"
#include "sequences.h"
#include "state.h"
#include "str.h"
#include "terminal.h"

const char *INVOCATION_NAME;

State state;

struct termios old_termios;
struct termios raw_termios;
bool raw_mode = false;

extern char error_message[ERROR_MESSAGE_LEN];

void cleanup(void);
int read_files(State *state,
               unsigned int n_positional_args, char **positional_args,
               unsigned int n_format_args, char **format_args,
               unsigned int n_type_args, char **type_args);

// --help option shows in given order (alphabetical except help and version)
Option options[] = {
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
// TODO: Possible --no-ascii option?

#define NOPTIONS sizeof(options) / sizeof(Option)

FormatOption format_options[] = {
    {"FASTA", "fasta,fa,faa,fna,afa", &fasta_fread},
    // CLUSTAL
    // PHYLIP
    // STOCKHOLM
};

#define N_FORMAT_OPTIONS sizeof(format_options) / sizeof(FormatOption)

SeqTypeOption type_options[] = {
    {"nucleic", "nucleic,nt", SEQ_TYPE_NUCLEIC, &NUCLEIC_ALPHABET},
    {"protein", "protein,aa", SEQ_TYPE_PROTEIN, &PROTEIN_ALPHABET},
};

#define N_TYPE_OPTIONS sizeof(type_options) / sizeof(SeqTypeOption)

SeqTypeState types[SEQ_TYPE_ERROR + 1];

// Main
int main(int argc, char *argv[])
{
    // Get invocation name
    if (argv[0] != NULL)
    {
        char *s = strrchr(argv[0], '/');
        if (s == NULL)
            INVOCATION_NAME = argv[0];
        else
            INVOCATION_NAME = s + 1; // Exclude /
    }
    else
        INVOCATION_NAME = "?";

    // Initializations
    int code = 0; // Generic return code for various functions
    atexit(&cleanup);
    if (sequences_init_base_alphabets() != 0)
    {
        snprintf(error_message, ERROR_MESSAGE_LEN, "%s: Failed to initialize alphabets\n", INVOCATION_NAME);
        return 1;
    };
    if (schemes_init_base() != 0)
    {
        snprintf(error_message, ERROR_MESSAGE_LEN, "%s: Failed to initialize color schemes\n", INVOCATION_NAME);
        return 1;
    }

    // Prepare color schemes
    state.color_schemes = schemes_base;
    state.n_color_schemes = SCHEMES_N_BASE_4_BIT;

    // Prepare known types
    for (unsigned int i = 0; i < N_TYPE_OPTIONS; i++)
    {
        SeqTypeOption *type_option = type_options + i;
        SeqTypeState *type = types + type_option->type;
        type->alphabet = type_option->alphabet;
    }
    state.types = types;
    state.ntypes = SEQ_TYPE_ERROR + 1;

    // Get terminal color support
    state.ncolors = 1;
    int errret;
    if (setupterm(NULL, STDOUT_FILENO, &errret) == OK) // Need to set a return code address; otherwise error will cause exit
    {
        int ncolors = tigetnum("colors");
        if (ncolors > 0)
            state.ncolors = ncolors;
    }

    // Set color schemes
    if (state.ncolors >= 256)
    {
        state_set_type_color_scheme(&state, SEQ_TYPE_NUCLEIC, &schemes_default_nucleic_8_bit);
        state_set_type_color_scheme(&state, SEQ_TYPE_PROTEIN, &schemes_default_protein_8_bit);
    }
    else if (state.ncolors >= 16)
    {
        state_set_type_color_scheme(&state, SEQ_TYPE_NUCLEIC, &schemes_default_nucleic_4_bit);
        state_set_type_color_scheme(&state, SEQ_TYPE_PROTEIN, &schemes_default_protein_4_bit);
    }

    // Prepare options
    struct option long_options[NOPTIONS + 1]; // Extra struct of 0s to mark end
    char *short_options = NULL;
    code = prepare_options(NOPTIONS, options, &short_options, long_options);
    if (code != 0)
        return code;

    // Parse options
    unsigned int n_format_args = 0;
    char **format_args = NULL;
    unsigned int n_type_args = 0;
    char **type_args = NULL;
    code = parse_options(argc, argv,
                         NOPTIONS, options,
                         N_FORMAT_OPTIONS, format_options,
                         N_TYPE_OPTIONS, type_options,
                         short_options, long_options,
                         &n_format_args, &format_args,
                         &n_type_args, &type_args);
    free(short_options);
    if (code > 0) // "Expected" exit == 1 and "unexpected" exit > 1; shift -1 for CLI convention
        return code - 1;
    unsigned int n_positional_args = argc - optind;
    char **positional_args = argv + optind;

    // Check for positional arguments
    if (isatty(STDIN_FILENO) && n_positional_args == 0)
    {
        print_short_help(NOPTIONS, options);
        return 1;
    }

    // Handle special cases for piped input
    unsigned int nfiles = n_positional_args;
    int input_fd;
    if (!isatty(STDIN_FILENO))
    {
        if (n_positional_args == 0)
            nfiles++; // If not a tty, treat stdin as an implicit first file
        input_fd = open("/dev/tty", O_RDONLY);
        if (input_fd == -1)
        {
            snprintf(error_message, ERROR_MESSAGE_LEN,
                     "%s: Failed to open /dev/tty for reading commands\n", INVOCATION_NAME);
            return 1;
        }
        TERMINAL_FILENO = input_fd;
    }
    else
        input_fd = STDIN_FILENO;

    // Read files
    FileState *files = malloc(nfiles * sizeof(FileState));
    if (files == NULL)
    {
        snprintf(error_message, ERROR_MESSAGE_LEN, "%s: Failed to allocate memory to load files\n", INVOCATION_NAME);
        return 1;
    }
    state.files = files;
    state.nfiles = nfiles;
    state.active_file = files;
    state.active_file_index = 0;

    code = read_files(&state,
                      n_positional_args, positional_args,
                      n_format_args, format_args,
                      n_type_args, type_args);
    if (code > 0)
        return code - 1;

    if (n_format_args > 0)
        str_free_split(format_args, n_format_args);
    if (n_type_args > 0)
        str_free_split(type_args, n_type_args);

    // Set screen and terminal options
    if (terminal_get_termios(&old_termios) != 0)
    {
        snprintf(error_message, ERROR_MESSAGE_LEN, "%s: Failed to get current termios\n", INVOCATION_NAME);
        return 1;
    }
    if (terminal_enable_raw_mode(&old_termios, &raw_termios) != 0)
    {
        snprintf(error_message, ERROR_MESSAGE_LEN, "%s: Failed to set raw mode\n", INVOCATION_NAME);
        return 1;
    };
    raw_mode = true;
    terminal_use_alternate_buffer();

    setlocale(LC_ALL, ""); // Necessary for wcswidth calls

    // Main loop
    int action;
    Array buffer;
    array_init(&buffer, sizeof(char));

    while (1)
    {
        action = input_get_action(input_fd);
        input_process_action(action, &buffer);

        display_refresh(&buffer);

        input_buffer_flush(&buffer);
    }
}

void cleanup(void)
{
    // Free memory
    for (unsigned int i = 0; i < state.n_color_schemes; i++)
        color_free_color_scheme(state.color_schemes + i);
    for (unsigned int i = 0; i < state.nfiles; i++)
        sequences_free_seq_records(state.files[i].records, state.files[i].nrecords); // Null if unset, so always safe to free
    free(state.files);

    // Restore terminal options
    if (raw_mode)
    {
        terminal_use_normal_buffer();
        terminal_disable_raw_mode(&old_termios);
    }
    if (TERMINAL_FILENO != STDIN_FILENO)
        close(TERMINAL_FILENO);

    // Print error
    if (error_message[0] != '\0')
        fputs(error_message, stderr);
}

int read_files(State *state,
               unsigned int n_positional_args, char **positional_args,
               unsigned int n_format_args, char **format_args,
               unsigned int n_type_args, char **type_args)
{
    int code = 0;

    // Split format extensions
    StrArray *formats_exts = malloc(N_FORMAT_OPTIONS * sizeof(StrArray));
    if (formats_exts == NULL)
    {
        snprintf(error_message, ERROR_MESSAGE_LEN,
                 "%s: Failed to allocate memory to split format extensions\n", INVOCATION_NAME);
        return 1;
    }
    for (unsigned int i = 0; i < N_FORMAT_OPTIONS; i++)
    {
        StrArray *format_exts = formats_exts + i;
        format_exts->data = NULL;
        format_exts->len = 0;
    }
    for (unsigned int i = 0; i < N_FORMAT_OPTIONS; i++)
    {
        FormatOption *format_option = format_options + i;
        StrArray *format_exts = formats_exts + i;
        format_exts->len = str_split(&format_exts->data, format_option->exts, ',');
    }

    // Split type identifiers
    StrArray *types_identifiers = malloc(N_TYPE_OPTIONS * sizeof(StrArray));
    if (formats_exts == NULL)
    {
        snprintf(error_message, ERROR_MESSAGE_LEN,
                 "%s: Failed to allocate memory to split type identifiers\n", INVOCATION_NAME);
        return 1;
    }
    for (unsigned int i = 0; i < N_TYPE_OPTIONS; i++)
    {
        StrArray *type_identifier = types_identifiers + i;
        type_identifier->data = NULL;
        type_identifier->len = 0;
    }
    for (unsigned int i = 0; i < N_TYPE_OPTIONS; i++)
    {
        SeqTypeOption *type_option = type_options + i;
        StrArray *type_identifiers = types_identifiers + i;
        type_identifiers->len = str_split(&type_identifiers->data, type_option->identifiers, ',');
    }

    // Main loop
    for (unsigned int file_index = 0; file_index < state->nfiles; file_index++)
    {
        const char *file_path, *file_ext;
        if (!isatty(STDIN_FILENO) && n_positional_args == 0)
            file_path = "-";
        else
            file_path = positional_args[file_index];

        // Infer reader
        char *format_arg = "";
        if (file_index < n_format_args)
            format_arg = format_args[file_index];
        size_t (*reader)(FILE *, SeqRecord **) = NULL;

        if (format_arg[0] != '\0') // From format argument
        {
            for (unsigned int i = 0; i < N_FORMAT_OPTIONS; i++)
            {
                FormatOption *format_option = format_options + i;
                StrArray *format_exts = formats_exts + i;
                if (str_is_in((const char **)format_exts->data, format_exts->len, format_arg)) // Cast to silence warning
                {
                    reader = format_option->reader;
                    break;
                }
                snprintf(error_message, ERROR_MESSAGE_LEN,
                         "%s: %s: Error identifying format\n", INVOCATION_NAME, format_arg);
                code = 2;
                goto cleanup;
            }
        }
        else if ((file_ext = strrchr(file_path, '.')) != NULL) // From path extension
        {
            file_ext++; // Exclude dot from comparison
            for (unsigned int i = 0; i < N_FORMAT_OPTIONS; i++)
            {
                FormatOption *format_option = format_options + i;
                StrArray *format_exts = formats_exts + i;
                if (str_is_in((const char **)format_exts->data, format_exts->len, file_ext)) // Cast to silence warning
                {
                    reader = format_option->reader;
                    break;
                }
                snprintf(error_message, ERROR_MESSAGE_LEN, "%s: %s: Unknown extension\n", INVOCATION_NAME, file_path);
                code = 2;
                goto cleanup;
            }
        }
        else
        {
            snprintf(error_message, ERROR_MESSAGE_LEN,
                     "%s: %s: No format or known extension\n", INVOCATION_NAME, file_path);
            code = 2;
            goto cleanup;
        }

        // Read file
        FILE *fp;
        if (strcmp(file_path, "-") == 0)
            fp = stdin;
        else if ((fp = fopen(file_path, "r")) == NULL)
        {
            snprintf(error_message, ERROR_MESSAGE_LEN, "%s: %s: %s\n", INVOCATION_NAME, file_path, strerror(errno));
            code = 1;
            goto cleanup;
        }
        SeqRecord *records = NULL;
        size_t nrecords = reader(fp, &records);
        if (nrecords < 0)
        {
            snprintf(error_message, ERROR_MESSAGE_LEN,
                     "%s: %s: Error processing file (code %zu)\n", INVOCATION_NAME, file_path, nrecords);
            code = 1;
            goto cleanup;
        }

        // Get maxlen
        size_t maxlen = 0;
        for (size_t i = 0; i < nrecords; i++)
        {
            SeqRecord *record = records + i;
            if (record->len > maxlen)
                maxlen = record->len;
        }

        // Set sequence type
        char *type_arg = "";
        if (file_index < n_type_args)
            type_arg = type_args[file_index];
        for (size_t i = 0; i < nrecords; i++)
        {
            SeqRecord *record = records + i;
            if (sequences_infer_seq_type(record) != 0)
            {
                snprintf(error_message, ERROR_MESSAGE_LEN,
                         "%s: %s: Error identifying sequence types\n", INVOCATION_NAME, file_path);
                code = 1;
                goto cleanup;
            };
            if (type_arg[0] != '\0')
            {
                for (unsigned int j = 0; j < N_TYPE_OPTIONS; j++)
                {
                    SeqTypeOption *type_option = type_options + j;
                    StrArray *type_identifiers = types_identifiers + j;
                    if (str_is_in((const char **)type_identifiers->data, type_identifiers->len, type_arg)) // Cast to silence warning
                    {
                        SeqType type = type_option->type;
                        if (record->type == SEQ_TYPE_INDETERMINATE && type == SEQ_TYPE_PROTEIN)
                            record->type = SEQ_TYPE_PROTEIN;
                        break;
                    }
                }
            }
            else if (record->type == SEQ_TYPE_INDETERMINATE && record->len >= rcparams_nucleic_tiebreak_len)
                record->type = SEQ_TYPE_NUCLEIC;
        }

        FileState *file = state->files + file_index;
        file->file_path = file_path;
        file->records = records;
        file->nrecords = nrecords;
        file->records_offset = 1;
        file->records_maxlen = maxlen;
        file->header_pane_width = rcparams_header_pane_width;
        file->ruler_pane_height = rcparams_ruler_pane_height;
        file->tick_spacing = rcparams_tick_spacing;
        file->offset_record = 0;
        file->offset_header = 0;
        file->offset_sequence = 0;
        file->cursor_record_i = 0;
        file->cursor_header_j = 0;
        file->cursor_sequence_j = 0;
    }

cleanup:
    for (unsigned int i = 0; i < N_FORMAT_OPTIONS; i++)
    {
        StrArray *format_exts = formats_exts + i;
        str_free_split(format_exts->data, format_exts->len);
        format_exts->data = NULL;
        format_exts->len = 0;
    }
    free(formats_exts);
    for (unsigned int i = 0; i < N_TYPE_OPTIONS; i++)
    {
        StrArray *type_identifiers = types_identifiers + i;
        str_free_split(type_identifiers->data, type_identifiers->len);
        type_identifiers->data = NULL;
        type_identifiers->len = 0;
    }
    free(types_identifiers);
    return code;
}
