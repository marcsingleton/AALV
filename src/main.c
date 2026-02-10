#include <fcntl.h>
#include <getopt.h>
#include <locale.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>
#include <unistd.h>

#include <curses.h>
#include <term.h> // Must follow curses

#include "action.h"
#include "argparse.h"
#include "array.h"
#include "cmd.h"
#include "display.h"
#include "error.h"
#include "fasta.h"
#include "input.h"
#include "macros.h"
#include "rcparams.h"
#include "schemes.h"
#include "scroller.h"
#include "sequences.h"
#include "state.h"
#include "str.h"
#include "terminal.h"

const char *INVOCATION_NAME;

State state;

struct termios old_termios;
struct termios raw_termios;
bool raw_mode = false;

void cleanup(void);
void handle_sigwinch(int signum);
int load_seqs(FileState *file, const char *format_arg, const char *seq_type_arg);
FileReader get_reader(FileState *file, const char *format_arg);
int set_seq_types(FileState *file, const char *seq_type_arg);
int set_unaligned_indices(FileState *file);

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

#define NOPTIONS sizeof(options) / sizeof(Option)

FormatOption format_options[] = {
    {"FASTA", "fasta,fa,faa,fna,afa", &fasta_fread},
    {"A2M/A3M", "a2m,a3m", &fasta_fread},
    // CLUSTAL
    // PHYLIP
    // STOCKHOLM
};

#define N_FORMAT_OPTIONS sizeof(format_options) / sizeof(FormatOption)

SeqTypeOption seq_type_options[] = {
    {"nucleic", "nucleic,nt", SEQ_TYPE_NUCLEIC, &NUCLEIC_ALPHABET},
    {"protein", "protein,aa", SEQ_TYPE_PROTEIN, &PROTEIN_ALPHABET},
};

#define N_SEQ_TYPE_OPTIONS sizeof(seq_type_options) / sizeof(SeqTypeOption)

char *option_delim = ",";

SeqTypeState seq_types[SEQ_TYPE_ERROR + 1];

char synopsis[] = PROGRAM_NAME " is a vim-inspired alignment viewer\n";
char positional_usage[] = "[<file> ...]";

// Main
int main(int argc, char *argv[])
{
    int retcode = 0; // Generic return code for various functions
    atexit(&cleanup);

    // Get invocation name
    if (argv[0])
    {
        char *s = strrchr(argv[0], '/');
        if (!s)
            INVOCATION_NAME = argv[0];
        else
            INVOCATION_NAME = s + 1; // Exclude /
    }
    else
        INVOCATION_NAME = "?";

    // Register window change handler
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_handler = handle_sigwinch;
    sa.sa_flags = 0; // No SA_RESTART--blocking read calls will return
    if (sigaction(SIGWINCH, &sa, NULL) == -1)
    {
        error_printf("%s: Failed to register signal handler\n", INVOCATION_NAME);
        return 1;
    }

    // Initializations
    if (sequences_init_base_alphabets() != 0)
    {
        error_printf("%s: Failed to initialize alphabets\n", INVOCATION_NAME);
        return 1;
    };
    if (schemes_init_base() != 0)
    {
        error_printf("%s: Failed to initialize color schemes\n", INVOCATION_NAME);
        return 1;
    }

    // Prepare color schemes
    state.color_schemes = schemes_base;
    state.n_color_schemes = SCHEMES_N_BASE_4_BIT;

    // Prepare known sequence types
    for (unsigned int i = 0; i < N_SEQ_TYPE_OPTIONS; i++)
    {
        SeqTypeOption *seq_type_option = seq_type_options + i;
        SeqTypeState *type = seq_types + seq_type_option->type;
        type->alphabet = seq_type_option->alphabet;
    }
    state.seq_types = seq_types;
    state.n_seq_types = SEQ_TYPE_ERROR + 1;

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
        state_set_seq_type_color_scheme(&state, SEQ_TYPE_NUCLEIC, &schemes_default_nucleic_8_bit);
        state_set_seq_type_color_scheme(&state, SEQ_TYPE_PROTEIN, &schemes_default_protein_8_bit);
    }
    else if (state.ncolors >= 16)
    {
        state_set_seq_type_color_scheme(&state, SEQ_TYPE_NUCLEIC, &schemes_default_nucleic_4_bit);
        state_set_seq_type_color_scheme(&state, SEQ_TYPE_PROTEIN, &schemes_default_protein_4_bit);
    }

    // Prepare options
    struct option long_options[NOPTIONS + 1]; // Extra struct of 0s to mark end
    char *short_options = NULL;
    retcode = cli_prepare_options(NOPTIONS, options, &short_options, long_options, INVOCATION_NAME);
    if (retcode > 0)
        return retcode;

    // Parse options
    unsigned int n_format_args = 0;
    char **format_args = NULL;
    unsigned int n_seq_type_args = 0;
    char **seq_type_args = NULL;
    retcode = argparse_options(argc, argv,
                               NOPTIONS, options,
                               N_FORMAT_OPTIONS, format_options,
                               N_SEQ_TYPE_OPTIONS, seq_type_options,
                               short_options, long_options,
                               &n_format_args, &format_args,
                               &n_seq_type_args, &seq_type_args,
                               PROGRAM_NAME, positional_usage, synopsis);
    free(short_options);
    if (retcode > 0) // "Expected" exit == 1 and "unexpected" exit > 1; shift -1 for CLI convention
        return retcode - 1;
    unsigned int n_positional_args = argc - optind;
    char **positional_args = argv + optind;

    // Check for positional arguments
    if (isatty(STDIN_FILENO) && n_positional_args == 0)
    {
        cli_print_short_help(NOPTIONS, options, PROGRAM_NAME, positional_usage);
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
            error_printf("%s: Failed to open /dev/tty for reading commands\n", INVOCATION_NAME);
            return 1;
        }
        TERMINAL_FILENO = input_fd;
    }
    else
        input_fd = STDIN_FILENO;

    // Allocate file states
    FileState *files = malloc(nfiles * sizeof(FileState));
    if (!files)
    {
        error_printf("%s: Failed to allocate memory to load files\n", INVOCATION_NAME);
        return 1;
    }
    state.files = files;
    state.nfiles = nfiles;
    state.active_file = files;
    state_set_active_file_index(&state, 0);

    // Initialize file states
    state_set_terminal_size(&state);

    for (unsigned int file_index = 0; file_index < state.nfiles; file_index++)
    {
        FileState *file = state.files + file_index;
        if (!isatty(STDIN_FILENO) && n_positional_args == 0)
            file->file_path = "-";
        else
            file->file_path = positional_args[file_index];

        scroller_init(&file->layout.scroller, SCROLLER_NPANES);
        scroller_set_active_pane(&file->layout.scroller, SCROLLER_SEQUENCE_PANE);
        state_set_active_file_index(&state, file_index);
        state_set_layout(&state, rcparams_ruler_records_divider_i, rcparams_header_sequence_divider_j);
        state_set_tick_spacing(&state, rcparams_tick_spacing);

        char *format_arg = "";
        if (file_index < n_format_args)
            format_arg = format_args[file_index];

        char *seq_type_arg = "";
        if (file_index < n_seq_type_args)
            seq_type_arg = seq_type_args[file_index];

        retcode = load_seqs(file, format_arg, seq_type_arg);
        if (retcode > 0) // "Expected" exit == 1 and "unexpected" exit > 1; shift -1 for CLI convention
        {
            error_printf("%s: Failed to load sequences in %s\n", INVOCATION_NAME, file->file_path);
            return retcode - 1;
        }
        retcode = set_unaligned_indices(file);
        if (retcode > 0)
        {
            error_printf("%s: Failed to allocate unaligned indices in %s\n", INVOCATION_NAME, file->file_path);
            return 1;
        }
    }
    state_set_active_file_index(&state, 0);

    if (n_format_args > 0)
        str_free_split(format_args, n_format_args);
    if (n_seq_type_args > 0)
        str_free_split(seq_type_args, n_seq_type_args);

    // Set screen and terminal options
    if (terminal_get_termios(&old_termios) != 0)
    {
        error_printf("%s: Failed to get current termios\n", INVOCATION_NAME);
        return 1;
    }
    raw_termios = old_termios; // Copy current settings to raw
    if (terminal_enable_raw_mode(&raw_termios) != 0)
    {
        error_printf("%s: Failed to set raw mode\n", INVOCATION_NAME);
        return 1;
    };
    raw_mode = true;
    terminal_set_blocking_read();
    terminal_use_alternate_buffer();

    // Main loop
    size_t count;
    Action action;

    Array read_buffer, write_buffer;
    array_init(&read_buffer, sizeof(char));
    array_init(&write_buffer, sizeof(char));

    while (1)
    {
        display_refresh(&write_buffer);
        input_write_buffer_flush(&write_buffer);

        switch (state.mode)
        {
        case NORMAL:
        {
            input_read_key(&read_buffer, input_fd);

            retcode = input_parse_keys(&read_buffer, &action, &count);
            switch (retcode)
            {
            case PARSE_SUCCESS:
                input_execute_action(&action, count);
                input_read_buffer_flush(&read_buffer);
                break;
            case PARSE_INCOMPLETE:
                break;
            case PARSE_FAIL:
                input_read_buffer_flush(&read_buffer);
                break;
            }
            break;
        }
        case COMMAND:
        {
            state.mode = NORMAL;
            state.refresh_command_pane = true;
            char *command = cmd_read_command(":");
            retcode = cmd_parse_and_execute_command(command);
        }
        }
    }
}

void cleanup(void)
{
    // Free memory
    sequences_deinit_base_alphabets();
    for (unsigned int i = 0; i < state.n_color_schemes; i++)
        color_deinit_color_scheme(state.color_schemes + i);
    if (state.files)
    {
        for (unsigned int i = 0; i < state.nfiles; i++)
        {
            scroller_deinit(&state.files[i].layout.scroller);
            sequences_deinit_seq_record_array(&state.files[i].record_array);
            sequences_deinit_unaligned_indices_array(&state.files[i].indices_array);
        }
        free(state.files);
    }

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

void handle_sigwinch(int signum)
{
    (void)signum; // Suppress unused parameter warning
    state.refresh_window = true;
}

int load_seqs(FileState *file, const char *format_arg, const char *seq_type_arg)
{
    int retcode;

    // Infer reader
    FileReader reader = get_reader(file, format_arg);
    if (!reader)
        return 2;

    // Read file
    FILE *fp;
    if (strcmp(file->file_path, "-") == 0)
        fp = stdin;
    else if (!(fp = fopen(file->file_path, "r")))
    {
        error_printf("%s: %s: %s\n", INVOCATION_NAME, file->file_path, strerror(errno));
        return 2;
    }
    SeqRecordArray *record_array = &file->record_array;
    retcode = reader(fp, record_array);
    if (retcode > 0)
    {
        error_printf("%s: %s: Error processing file (code %d)\n", INVOCATION_NAME, file->file_path, retcode);
        return 2;
    }

    // Get maxlen
    size_t maxlen = 0;
    for (size_t i = 0; i < record_array->len; i++)
    {
        SeqRecord *record = record_array->data + i;
        if (record->len > maxlen)
            maxlen = record->len;
    }

    // Set sequence type
    retcode = set_seq_types(file, seq_type_arg);
    if (retcode > 0)
        return 1;

    file->tick_offset = 1;
    file->records_maxlen = maxlen;

    return 0;
}

FileReader get_reader(FileState *file, const char *format_arg)
{
    const char *file_ext;
    if (format_arg[0] != '\0') // From format argument
    {
        for (unsigned int i = 0; i < N_FORMAT_OPTIONS; i++)
        {
            FormatOption *format_option = format_options + i;
            const char *exts = format_option->exts;
            if (str_is_in_strsep(exts, option_delim, format_arg))
                return format_option->reader;
            error_printf("%s: %s: Error identifying format\n", INVOCATION_NAME, format_arg);
            return NULL;
        }
    }
    else if ((file_ext = strrchr(file->file_path, '.'))) // From path extension
    {
        file_ext++; // Exclude dot from comparison
        for (unsigned int i = 0; i < N_FORMAT_OPTIONS; i++)
        {
            FormatOption *format_option = format_options + i;
            const char *exts = format_option->exts;
            if (str_is_in_strsep(exts, option_delim, file_ext))
                return format_option->reader;
            break;
            error_printf("%s: %s: Unknown extension\n", INVOCATION_NAME, file->file_path);
            return NULL;
        }
    }
    error_printf("%s: %s: No format or known extension\n", INVOCATION_NAME, file->file_path);
    return NULL;
}

int set_seq_types(FileState *file, const char *seq_type_arg)
{
    SeqRecordArray *record_array = &(file->record_array);
    for (size_t i = 0; i < record_array->len; i++)
    {
        SeqRecord *record = record_array->data + i;
        SeqType seq_type = sequences_seq_to_seq_type(record->seq);
        if (seq_type == SEQ_TYPE_ERROR)
        {
            printf("%s contains at least one non-ASCII symbol in its sequence(s). "
                   "The viewer may render incorrectly. Continue? (y/n): ",
                   file->file_path);
            int c = getchar();
            if (c != 'y' && c != 'Y')
                return 1;
            while ((c = getchar()) != '\n' && c != EOF)
                ; // Clear remaining input
        };
        if (seq_type_arg[0] != '\0')
        {
            for (unsigned int j = 0; j < N_SEQ_TYPE_OPTIONS; j++)
            {
                SeqTypeOption *seq_type_option = seq_type_options + j;
                const char *identifiers = seq_type_option->identifiers;
                if (str_is_in_strsep(identifiers, option_delim, seq_type_arg))
                {
                    if (seq_type != SEQ_TYPE_ERROR) // Allow forced type unless error
                        record->type = seq_type;
                    break;
                }
            }
        }
        else if (seq_type == SEQ_TYPE_RNA || seq_type == SEQ_TYPE_DNA)
            record->type = SEQ_TYPE_NUCLEIC;
        else if (seq_type == SEQ_TYPE_INDETERMINATE && record->len >= rcparams_nucleic_tiebreak_len)
            record->type = SEQ_TYPE_NUCLEIC;
        else
            record->type = seq_type;
    }
    return 0;
}

int set_unaligned_indices(FileState *file)
{
    int retcode = sequences_init_unaligned_indices_array(&file->indices_array, file->record_array.len);
    if (retcode > 0)
        return 1;
    for (size_t i = 0; i < file->record_array.len; i++)
    {
        SeqRecord *record = file->record_array.data + i;
        UnalignedIndices *unaligned_indices = file->indices_array.data + i;
        Alphabet *alphabet = sequences_seq_type_to_alphabet(record->type);
        if (alphabet)
            sequences_index_nongap_syms(alphabet, record, unaligned_indices);
    }
    return 0;
}
