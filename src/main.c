#include <stdio.h>

#include "array.h"
#include "fasta.h"
#include "seqrecord.h"

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
    FILE *alignment = fopen(argv[1], "r");
    if (alignment == NULL)
    {
        fputs("Failed to open file. Quitting...\n", stderr);
        return 1;
    }

    SeqRecord *records = NULL;
    int nrecords = fasta_parse(alignment, &records);

    // Main loop
    // Display current file
    // Read input
    // Process input
    for (int i = 0; i < nrecords; i++)
    {
        printf("\nRecord %d\n", i);
        printf("%s\n", records[i].header);
        printf("%s\n", records[i].seq);
    }

    // Clean up
    // Free memory and restore terminal options
    fclose(alignment);
    return 0;
}
