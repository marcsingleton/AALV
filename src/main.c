#include <stdio.h>
#include <stdlib.h>

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

    // Main loop
    // Display current file
    // Read input
    // Process input
    char *line = NULL;
    size_t capacity = 0;
    ssize_t linelen;
    while ((linelen = getline(&line, &capacity, alignment)) > 0)
    {
        fputs(line, stdout);
    }

    // Clean up
    // Free memory and restore terminal options
    free(line);
    fclose(alignment);
    return 0;
}
