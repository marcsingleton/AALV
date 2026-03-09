#include <fcntl.h>
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
#include "config.h"
#include "display.h"
#include "error.h"
#include "fasta.h"
#include "input.h"
#include "macros.h"
#include "schemes.h"
#include "scroller.h"
#include "sequences.h"
#include "state.h"
#include "str.h"
#include "terminal.h"

const char *INVOCATION_NAME;

State state;

Array read_buffer;
Array write_buffer;
char *cmd_line;

struct termios old_termios;
struct termios raw_termios;
bool raw_mode = false;

void cleanup(void);
void handle_sigwinch(int signum);
int load_user_config(void);
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

unsigned int noptions = sizeof(options) / sizeof(Option);

FormatOption format_options[] = {
    {"FASTA", "fasta,fa,faa,fna,afa", &fasta_fread},
    {"A2M/A3M", "a2m,a3m", &fasta_fread},
    // CLUSTAL
    // PHYLIP
    // STOCKHOLM
};

unsigned int n_format_options = sizeof(format_options) / sizeof(FormatOption);

SeqTypeOption seq_type_options[] = {
    {"nucleic", "nucleic,nt", SEQ_TYPE_NUCLEIC, &NUCLEIC_ALPHABET},
    {"protein", "protein,aa", SEQ_TYPE_PROTEIN, &PROTEIN_ALPHABET},
};

unsigned int n_seq_type_options = sizeof(seq_type_options) / sizeof(SeqTypeOption);

char *option_delim = ",";

SeqColorScheme *active_color_schemes[SEQ_TYPE_ERROR + 1];

unsigned int n_active_color_schemes = sizeof(active_color_schemes) / sizeof(SeqColorScheme *);

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
        return EXIT_FAILURE;
    }

    // Initialize globals
    if (sequences_init_base_alphabets() != 0)
    {
        error_printf("%s: Failed to initialize alphabets\n", INVOCATION_NAME);
        return EXIT_FAILURE;
    };
    if (schemes_init_base() != 0)
    {
        error_printf("%s: Failed to initialize base color schemes\n", INVOCATION_NAME);
        return EXIT_FAILURE;
    }
    if (cmd_init_command_map() != 0)
    {
        error_printf("%s: Failed to initialize command map\n", INVOCATION_NAME);
        return EXIT_FAILURE;
    }
    if (array_init(&read_buffer, sizeof(char)) != 0)
    {
        error_printf("%s: Failed to initialize read buffer\n", INVOCATION_NAME);
        return EXIT_FAILURE;
    };
    if (array_init(&write_buffer, sizeof(char)) != 0)
    {
        error_printf("%s: Failed to initialize write buffer\n", INVOCATION_NAME);
        return EXIT_FAILURE;
    };

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
    state.active_color_schemes = active_color_schemes;
    state.n_active_color_schemes = n_active_color_schemes;
    if (state.ncolors >= 256)
    {
        state_set_active_color_scheme(&state, &schemes_default_nucleic_8_bit);
        state_set_active_color_scheme(&state, &schemes_default_protein_8_bit);
    }
    else if (state.ncolors >= 16)
    {
        state_set_active_color_scheme(&state, &schemes_default_nucleic_4_bit);
        state_set_active_color_scheme(&state, &schemes_default_protein_4_bit);
    }

    // Prepare options
    struct option long_options[noptions + 1]; // Extra struct of 0s to mark end
    char *short_options = NULL;
    retcode = cli_prepare_options(noptions, options, &short_options, long_options, INVOCATION_NAME);
    if (retcode != 0)
        return retcode;

    // Parse options
    unsigned int n_format_args = 0;
    char **format_args = NULL;
    unsigned int n_seq_type_args = 0;
    char **seq_type_args = NULL;
    retcode = argparse_options(argc, argv,
                               noptions, options,
                               n_format_options, format_options,
                               n_seq_type_options, seq_type_options,
                               short_options, long_options,
                               &n_format_args, &format_args,
                               &n_seq_type_args, &seq_type_args,
                               PROGRAM_NAME, positional_usage, synopsis);
    free(short_options);
    if (retcode != 0) // "Expected" exit == -1 and "unexpected" exit < -1
        return (retcode == -1) ? EXIT_SUCCESS : EXIT_FAILURE;
    unsigned int n_positional_args = argc - optind;
    char **positional_args = argv + optind;

    // Check for positional arguments
    if (isatty(STDIN_FILENO) && n_positional_args == 0)
    {
        cli_print_short_help(noptions, options, PROGRAM_NAME, positional_usage);
        return EXIT_FAILURE;
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
            return EXIT_FAILURE;
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
        return EXIT_FAILURE;
    }
    state.files = files;
    state.nfiles = nfiles;
    state.active_file = files;
    state_set_active_file_index(&state, 0);

    // Initialize state and config
    state_set_terminal_size(&state);
    config_init();
    load_user_config();

    // Initialize file states
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
        state_set_layout(&state, config_ruler_records_divider, config_header_sequence_divider);
        state_set_tick_spacing(&state, config_tick_spacing);

        char *format_arg = "";
        if (file_index < n_format_args)
            format_arg = format_args[file_index];

        char *seq_type_arg = "";
        if (file_index < n_seq_type_args)
            seq_type_arg = seq_type_args[file_index];

        retcode = load_seqs(file, format_arg, seq_type_arg);
        if (retcode != 0) // "Expected" exit == -1 and "unexpected" exit < -1
        {
            return (retcode == -1) ? EXIT_SUCCESS : EXIT_FAILURE;
        }
        retcode = set_unaligned_indices(file);
        if (retcode != 0)
        {
            error_printf("%s: Failed to allocate unaligned indices in %s\n", INVOCATION_NAME, file->file_path);
            return EXIT_FAILURE;
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
        return EXIT_FAILURE;
    }
    raw_termios = old_termios; // Copy current settings to raw
    if (terminal_enable_raw_mode(&raw_termios) != 0)
    {
        error_printf("%s: Failed to set raw mode\n", INVOCATION_NAME);
        return EXIT_FAILURE;
    };
    raw_mode = true;
    terminal_set_blocking_read();
    terminal_use_alternate_buffer();

    // Main loop
    size_t count;
    Action action;

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
            cmd_line = cmd_read_command_line(input_fd, ":");
            retcode = cmd_parse_and_execute_command_line(cmd_line);
        }
        }
    }
}

void cleanup(void)
{
    // Free globals
    sequences_deinit_base_alphabets();
    schemes_deinit_base();
    cmd_deinit_command_map();
    array_deinit(&read_buffer);
    array_deinit(&write_buffer);
    free(cmd_line);

    // Free state
    if (state.color_schemes)
    {
        for (unsigned int i = 0; i < state.n_color_schemes; i++)
            color_deinit_color_scheme(&state.color_schemes[i].scheme);
        free(state.color_schemes);
    }
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

int load_user_config(void)
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

    size_t capacity = 0;
    ssize_t line_len = 0;
    while ((line_len = getline(&cmd_line, &capacity, config_fp)) > 0)
        cmd_parse_and_execute_command_line(cmd_line);
    return 0;
}

int load_seqs(FileState *file, const char *format_arg, const char *seq_type_arg)
{
    int retcode;

    // Infer reader
    FileReader reader = get_reader(file, format_arg);
    if (!reader)
        return -2;

    // Read file
    FILE *fp;
    if (strcmp(file->file_path, "-") == 0)
        fp = stdin;
    else if (!(fp = fopen(file->file_path, "r")))
    {
        error_printf("%s: %s: %s\n", INVOCATION_NAME, file->file_path, strerror(errno));
        return -2;
    }
    SeqRecordArray *record_array = &file->record_array;
    retcode = reader(fp, record_array);
    if (retcode != 0)
    {
        error_printf("%s: %s: Error processing file (code %d)\n", INVOCATION_NAME, file->file_path, retcode);
        return -2;
    }

    // Get max_len
    size_t max_len = 0;
    for (size_t i = 0; i < record_array->len; i++)
    {
        SeqRecord *record = record_array->data + i;
        if (record->len > max_len)
            max_len = record->len;
    }
    if (max_len > INT_MAX)
        return -2;

    // Set sequence type
    retcode = set_seq_types(file, seq_type_arg);
    if (retcode != 0)
        return -1;

    file->tick_offset = 1;
    file->records_max_len = max_len;

    return 0;
}

FileReader get_reader(FileState *file, const char *format_arg)
{
    const char *file_ext;
    if (format_arg[0] != '\0') // From format argument
    {
        for (unsigned int i = 0; i < n_format_options; i++)
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
        for (unsigned int i = 0; i < n_format_options; i++)
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
        SeqType seq_type_from_seq = sequences_seq_to_seq_type(record->seq);
        if (seq_type_from_seq == SEQ_TYPE_ERROR)
        {
            printf("%s contains at least one non-ASCII symbol in its sequence(s). "
                   "The viewer may render incorrectly. Continue? (y/n): ",
                   file->file_path);
            int c = getchar();
            if (c != 'y' && c != 'Y')
                return -1;
            while ((c = getchar()) != '\n' && c != EOF)
                ; // Clear remaining input
        };
        if (seq_type_from_seq != SEQ_TYPE_ERROR && seq_type_arg[0] != '\0') // Allow forced type unless error
        {
            SeqType seq_type_from_arg = argparse_seq_type(seq_type_arg, n_seq_type_options, seq_type_options);
            record->type = seq_type_from_arg;
        }
        else if (seq_type_from_seq == SEQ_TYPE_RNA || seq_type_from_seq == SEQ_TYPE_DNA)
            record->type = SEQ_TYPE_NUCLEIC;
        else if (seq_type_from_seq == SEQ_TYPE_INDETERMINATE && record->len >= config_nucleic_tiebreak_len)
            record->type = SEQ_TYPE_NUCLEIC;
        else
            record->type = seq_type_from_seq;
    }
    return 0;
}

int set_unaligned_indices(FileState *file)
{
    int retcode = sequences_init_unaligned_indices_array(&file->indices_array, file->record_array.len);
    if (retcode != 0)
        return -1;
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
