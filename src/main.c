#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "array.h"
#include "fasta.h"
#include "input.h"
#include "sequences.h"
#include "state.h"
#include "terminal.h"

State state;
void cleanup(void);

// Main
int main(int argc, char *argv[])
{
    // Process arguments
    if (argc != 2)
    {
        fputs("Incorrect number of arguments. Quitting...\n", stderr);
        return 1;
    }

    // Set screen and terminal options
    if (terminal_get_termios(&state.old_termios) != 0)
    {
        fputs("Failed to get current termios. Quitting...\n", stderr);
        return 1;
    }
    atexit(&cleanup); // Only register when get_termios is successful
    if (terminal_enable_raw_mode(&state.old_termios, &state.raw_termios) != 0)
    {
        fputs("Failed to set raw mode. Quitting...\n", stderr);
        return 1;
    };
    terminal_use_alternate_buffer();

    int rows, cols;
    terminal_get_window_size(&rows, &cols);

    // Read files
    // - Try to match extensions
    // - If no matching extension, infer with sniffer
    // - Error if no sniffer is successful
    // - Error if an explicit or inferred format is malformed

    SeqRecordArray record_array;
    SeqRecord *records = NULL;
    int len = fasta_read(argv[1], &records); // TODO: test for errors
    record_array.records = records;
    record_array.len = len;

    printf("Read %d records\r\n", record_array.len);
    for (int i = 0; i < record_array.len; i++)
    {
        SeqRecord *record = record_array.records + i;
        printf("Record %d\r\n", i);
        printf("\theader: %s\r\n\tid: %s\r\n\tseq: %s\r\n", record->header, record->id, record->seq);
    }

    // Main loop
    // Display current file
    // Read input
    // Process input
    int action;
    while (1)
    {
        action = input_get_action();
        input_process_action(action);
    }
}

void cleanup(void)
{
    // Free memory
    sequences_free_seq_record_array(&state.record_array); // Null if unset, so always safe to free

    // Restore terminal options
    terminal_use_normal_buffer();
    terminal_disable_raw_mode(&state.old_termios);
}
