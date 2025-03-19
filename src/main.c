#include <ctype.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>

#include "array.h"
#include "fasta.h"
#include "sequences.h"
#include "terminal.h"

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
    struct termios old_termios;
    struct termios raw_termios;
    terminal_enable_raw_mode(&old_termios, &raw_termios);

    // Read files
    // - Try to match extensions
    // - If no matching extension, infer with sniffer
    // - Error if no sniffer is successful
    // - Error if an explicit or inferred format is malformed

    SeqRecord *records = NULL;
    int nrecords = fasta_read(argv[1], &records);
    printf("%d\r\n", nrecords);
    for (int i = 0; i < nrecords; i++)
    {
        SeqRecord record = *(records + i);
        printf("%s\r\n", record.seq);
    }

    // Main loop
    // Display current file
    // Read input
    // Process input
    char c;
    while (read(STDIN_FILENO, &c, 1) == 1 && c != 'q')
    {
        if (iscntrl(c))
        {
            printf("%d\r\n", c);
        }
        else
        {
            printf("%d ('%c')\r\n", c, c);
        }
    }

    // Clean up
    // Free memory and restore terminal options
    terminal_disable_raw_mode(&old_termios);
    return 0;
}
