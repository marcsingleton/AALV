#include <signal.h>
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

State state;
extern char error_message[ERROR_MESSAGE_LEN];

void cleanup(void);
void handle_sigwinch(int signum);

// Main
int main(int argc, char *argv[])
{
    // Process arguments
    if (argc != 2)
    {
        strncpy(error_message, "Incorrect number of arguments. Quitting...\n", ERROR_MESSAGE_LEN);
        return 1;
    }

    // Set screen and terminal options
    if (terminal_get_termios(&state.old_termios) != 0)
    {
        strncpy(error_message, "Failed to get current termios. Quitting...\n", ERROR_MESSAGE_LEN);
        return 1;
    }
    atexit(&cleanup); // Only register when get_termios is successful
    if (terminal_enable_raw_mode(&state.old_termios, &state.raw_termios) != 0)
    {
        strncpy(error_message, "Failed to set raw mode. Quitting...\n", ERROR_MESSAGE_LEN);
        return 1;
    };
    terminal_use_alternate_buffer();

    terminal_get_window_size(&state.terminal_rows, &state.terminal_cols);
    signal(SIGWINCH, handle_sigwinch);

    // Read files
    // - Try to match extensions
    // - If no matching extension, infer with sniffer
    // - Error if no sniffer is successful
    // - Error if an explicit or inferred format is malformed
    SeqRecord *records = NULL;
    int len = fasta_read(argv[1], &records);
    if (len < 0)
    {
        snprintf(error_message, ERROR_MESSAGE_LEN, "Error processing input file: %d\n", len);
        return 1;
    }
    state.record_array.records = records;
    state.record_array.len = len;
    state.record_array.offset = 3;

    // Main loop
    // Display current file
    // Read input
    // Process input
    int action;
    Array buffer;
    array_init(&buffer, sizeof(char));

    state.header_pane_width = 30;
    state.ruler_pane_height = 5;
    state.tick_spacing = 10;
    state.offset_record = 0;
    state.offset_header = 0;
    state.offset_sequence = 0;
    state.cursor_record_i = 0;
    state.cursor_header_j = 0;
    state.cursor_sequence_j = 0;

    terminal_cursor_hide(&buffer);
    display_all_panes(&buffer);
    display_cursor(&buffer);
    display_command_pane(&buffer);
    terminal_cursor_show(&buffer);

    input_buffer_flush(&buffer);

    while (1)
    {
        action = input_get_action();
        input_process_action(action, &buffer);

        // Re-paint screen if necessary
        terminal_cursor_hide(&buffer);
        state.refresh_command_pane = true;
        if (state.refresh_window)
        {
            terminal_get_window_size(&state.terminal_rows, &state.terminal_cols);
            terminal_clear_screen(&buffer);
            state_set_header_pane_width(&state, state.header_pane_width);
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
    sequences_free_seq_record_array(&state.record_array); // Null if unset, so always safe to free

    // Restore terminal options
    terminal_use_normal_buffer();
    terminal_disable_raw_mode(&state.old_termios);

    // Print error
    if (error_message[0] != '\0')
        fputs(error_message, stderr);
}

void handle_sigwinch(int signum)
{
    (void)signum; // Suppress unused parameter warning
    state.refresh_window = true;
}
