#include <fcntl.h>
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
#include "input.h"
#include "io.h"
#include "macros.h"
#include "schemes.h"
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

volatile sig_atomic_t winch_flag;
volatile sig_atomic_t tstp_flag;
volatile sig_atomic_t cont_flag;

void cleanup(void);
void process_signal_flags(void);
void handle_sigwinch(int signum);
void handle_sigtstp(int signum);
void handle_sigcont(int signum);
int register_sigwinch_handler(void);
int register_sigtstp_handler(void);
int register_sigcont_handler(void);
int init_terminal(void);
int deinit_terminal(void);

int run_user_config(State *state);

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

    // Register handlers
    if (register_sigwinch_handler() == -1)
    {
        error_printf("%s: Failed to register WINCH signal handler\n", INVOCATION_NAME);
        return EXIT_FAILURE;
    }
    if (register_sigtstp_handler() == -1)
    {
        error_printf("%s: Failed to register TSTP signal handler\n", INVOCATION_NAME);
        return EXIT_FAILURE;
    }
    if (register_sigcont_handler() == -1)
    {
        error_printf("%s: Failed to register CONT signal handler\n", INVOCATION_NAME);
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

    // Check if connected to TTY
    int input_fd;
    if (!isatty(STDIN_FILENO))
    {
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

    // Initialize state
    state_init(&state);
    run_user_config(&state);

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

    // Initialize file states
    unsigned int nfiles = n_positional_args;
    if (!isatty(STDIN_FILENO) && n_positional_args == 0)
        nfiles++; // If not a tty and no args, treat stdin as an implicit first file

    if (isatty(STDIN_FILENO) && nfiles == 0)
    {
        if (state_new_file(&state) != 0)
        {
            error_printf("%s: Failed to allocate memory to load file\n", INVOCATION_NAME);
            return EXIT_FAILURE;
        }
        FileState *file = state.files;

        if (state_set_file_path(file, "") != 0)
        {
            error_printf("%s: Failed to allocate memory to store file path\n", INVOCATION_NAME);
            return EXIT_FAILURE;
        }
    }
    else
    {
        for (unsigned int file_index = 0; file_index < nfiles; file_index++)
        {
            // Make file
            state_new_file(&state);
            if (state.nfiles != file_index + 1)
            {
                error_printf("%s: Failed to allocate memory to load file\n", INVOCATION_NAME);
                return EXIT_FAILURE;
            }
            FileState *file = state.files + file_index;

            // Set file path
            char *file_path;
            if (!isatty(STDIN_FILENO) && n_positional_args == 0)
                file_path = "-";
            else
                file_path = positional_args[file_index];
            if (state_set_file_path(file, file_path) != 0)
            {
                error_printf("%s: Failed to allocate memory to store file path\n", INVOCATION_NAME);
                return EXIT_FAILURE;
            }

            // Get CLI args
            char *format_arg = "";
            if (file_index < n_format_args)
                format_arg = format_args[file_index];

            char *seq_type_arg = "";
            if (file_index < n_seq_type_args)
                seq_type_arg = seq_type_args[file_index];

            // Get reader
            if (format_arg[0] == '\0') // From format argument
            {
                char *ext = strrchr(file_path, '.');
                if (ext)
                    format_arg = ++ext; // Shift past dot
                else
                {
                    error_printf("%s: %s: No format argument or extension\n", INVOCATION_NAME, file->file_path);
                    return EXIT_FAILURE;
                }
            }
            FileReader reader = io_get_reader(format_arg, n_format_options, format_options);
            if (!reader)
            {
                error_printf("%s: %s: Unknown extension\n", INVOCATION_NAME, file->file_path);
                return EXIT_FAILURE;
            }

            // Open file
            FILE *fp;
            if (strcmp(file->file_path, "-") == 0)
                fp = stdin;
            else if (!(fp = fopen(file->file_path, "r")))
            {
                error_printf("%s: %s: %s\n", INVOCATION_NAME, file->file_path, strerror(errno));
                return EXIT_FAILURE;
            }

            // Load seqs
            retcode = io_load_seqs(file, fp, reader);
            if (retcode != 0)
            {
                error_printf("%s: Failed to load sequences in %s\n", INVOCATION_NAME, file->file_path);
                return EXIT_FAILURE;
            }

            // Close file
            retcode = fclose(fp);
            if (retcode != 0)
            {
                error_printf("%s: %s: %s\n", INVOCATION_NAME, file->file_path, strerror(errno));
                return EXIT_FAILURE;
            }

            // Set seq types
            retcode = io_set_seq_types(file, seq_type_arg, state.config.nucleic_tiebreak_len);
            if (retcode != 0)
            {
                error_printf("%s: Failed to set sequence types in %s\n", INVOCATION_NAME, file->file_path);
                return EXIT_FAILURE;
            }

            // Set indices
            retcode = io_set_unaligned_indices(file);
            if (retcode != 0)
            {
                error_printf("%s: Failed to allocate unaligned indices in %s\n", INVOCATION_NAME, file->file_path);
                return EXIT_FAILURE;
            }
        }
    }
    state_set_active_file_index(&state, 0);

    if (n_format_args > 0)
        str_free_split(format_args, n_format_args);
    if (n_seq_type_args > 0)
        str_free_split(seq_type_args, n_seq_type_args);

    // Set screen and terminal options
    retcode = init_terminal();
    switch (retcode)
    {
    case 0:
        break;
    case -1:
        error_printf("%s: Failed to get current termios\n", INVOCATION_NAME);
        return EXIT_FAILURE;
    case -2:
        error_printf("%s: Failed to set raw mode\n", INVOCATION_NAME);
        return EXIT_FAILURE;
    case -3:
        error_printf("%s: Failed to set blocking read\n", INVOCATION_NAME);
        return EXIT_FAILURE;
    default:
        error_printf("%s: Unknown error code during terminal initialization: %d\n", INVOCATION_NAME, retcode);
        return EXIT_FAILURE;
    }

    // Special case for splash screen
    if (isatty(STDIN_FILENO) && nfiles == 0)
    {
        display_refresh(&state, &write_buffer);
        input_write_buffer_flush(&write_buffer);
        display_splash_screen(&state, &write_buffer);
    }

    // Main loop
    size_t count;
    Action action;

    while (state.nfiles > 0)
    {
        display_refresh(&state, &write_buffer);
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
                input_execute_action(&state, &action, count);
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
            retcode = cmd_parse_and_execute_command_line(&state, cmd_line);
        }
        }

        process_signal_flags();
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
    state_deinit(&state);

    // Restore terminal options
    deinit_terminal();
    if (TERMINAL_FILENO != STDIN_FILENO)
        close(TERMINAL_FILENO);

    // Print error
    if (error_message[0] != '\0')
        fputs(error_message, stderr);
}

void process_signal_flags(void)
{
    if (winch_flag)
    {
        state.refresh_window = true;
        winch_flag = 0;
    }
    if (tstp_flag)
    {
        deinit_terminal();

        // Reset SIGTSTP to default
        struct sigaction sa;
        sa.sa_handler = SIG_DFL;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = SA_RESTART;
        sigaction(SIGTSTP, &sa, NULL);

        raise(SIGTSTP); // Re-raise to stop process
        tstp_flag = 0;
    }
    if (cont_flag)
    {
        // Re-register the SIGTSTP handler on resume
        register_sigtstp_handler();

        init_terminal();
        state.refresh_window = true;
        cont_flag = 0;
    }
}

void handle_sigwinch(int signum)
{
    (void)signum; // Suppress unused parameter warning
    winch_flag = 1;
}

void handle_sigtstp(int signum)
{
    (void)signum; // Suppress unused parameter warning
    tstp_flag = 1;
}

void handle_sigcont(int signum)
{
    (void)signum; // Suppress unused parameter warning
    cont_flag = 1;
}

int register_sigwinch_handler(void)
{
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_handler = handle_sigwinch;
    sa.sa_flags = 0; // No SA_RESTART--blocking read calls will return
    return sigaction(SIGWINCH, &sa, NULL);
}

int register_sigtstp_handler(void)
{
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sigaddset(&sa.sa_mask, SIGCONT);
    sa.sa_handler = handle_sigtstp;
    sa.sa_flags = 0; // No SA_RESTART--blocking read calls will return
    return sigaction(SIGTSTP, &sa, NULL);
}

int register_sigcont_handler(void)
{
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sigaddset(&sa.sa_mask, SIGTSTP);
    sa.sa_handler = handle_sigcont;
    sa.sa_flags = 0; // No SA_RESTART--blocking read calls will return
    return sigaction(SIGCONT, &sa, NULL);
}

int init_terminal(void)
{
    if (terminal_get_termios(&old_termios) != 0)
        return -1;
    raw_termios = old_termios; // Copy current settings to raw

    if (terminal_enable_raw_mode(&raw_termios) != 0)
        return -2;
    raw_mode = true; // Not re-entrant, but unlikely to cause issues during signal handling

    if (terminal_set_blocking_read() != 0)
        return -3;

    terminal_use_alternate_buffer();

    return 0;
}

int deinit_terminal(void)
{
    if (raw_mode)
    {
        terminal_use_normal_buffer();

        if (terminal_disable_raw_mode(&old_termios) != 0)
            return -1;
        raw_mode = false; // Not re-entrant, but unlikely to cause issues during signal handling
    }

    return 0;
}

int run_user_config(State *state)
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
    fclose(config_fp);

    return 0;
}
