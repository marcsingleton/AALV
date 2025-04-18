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
void cleanup(void);
extern char error_message[ERROR_MESSAGE_LEN];

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

    // Main loop
    // Display current file
    // Read input
    // Process input
    int action;
    Array buffer;
    array_init(&buffer, sizeof(char));

    state.header_pane_width = 30;
    state.ruler_pane_height = 5;
    state.cursor_i = state.ruler_pane_height + 1;
    state.cursor_j = state.header_pane_width + 1;

    terminal_cursor_hide(&buffer);
    display_ruler_pane(&buffer);
    display_header_pane(&buffer);
    display_sequence_pane(&buffer);
    display_cursor(&buffer);
    terminal_cursor_show(&buffer);

    input_buffer_flush(&buffer);

    while (1)
    {
        action = input_get_action();
        input_process_action(action, &buffer);
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
