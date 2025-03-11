#include <stdio.h>

#include "array.h"
#include "fasta.h"
#include "sequences.h"

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

    // Read files
    // - Try to match extensions
    // - If no matching extension, infer with sniffer
    // - Error if no sniffer is successful
    // - Error if an explicit or inferred format is malformed

    SeqRecord *records = NULL;
    int nrecords = fasta_read(argv[1], &records);
    printf("Records is: %d\n", nrecords);

    // Main loop
    // Display current file
    // Read input
    // Process input
    fasta_fwrite(stdout, records, nrecords, 10);

    // Clean up
    // Free memory and restore terminal options
    return 0;
}
