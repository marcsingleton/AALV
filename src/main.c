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

// Main
int main(int argc, char *argv[])
{
    // Process arguments
    if (argc < 2)
    {
        strncpy(error_message, PROGRAM_NAME ": Incorrect number of arguments\n", ERROR_MESSAGE_LEN);
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
    // - Try to match extensions
    // - If no matching extension, infer with sniffer
    // - Error if no sniffer is successful
    // - Error if an explicit or inferred format is malformed

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
        char *file_path = argv[i + 1];
        SeqRecord *records = NULL;
        int len = fasta_read(argv[i + 1], &records);
        if (len < 0)
        {
            snprintf(error_message, ERROR_MESSAGE_LEN, PROGRAM_NAME ": %s: Error processing file (code %d)\n", file_path, len);
            return 1;
        }
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
