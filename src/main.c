#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct
{
    char *seq;
    char *id;
    char *header;
    int len;
} SeqRecord;

typedef struct
{
    char *SeqRecord;
    int len;
} SeqRecordArray;

int parse_fasta(FILE *fp, SeqRecord **records_ptr)
{
    long pos;

    int nrecords = 0;
    char *line = NULL;
    size_t capacity;
    ssize_t linelen;

    if ((pos = ftell(fp)) == -1)
    {
        perror("ftell failed in parse_fasta.");
        exit(1);
    }

    while ((linelen = getline(&line, &capacity, fp)) > 0)
    {
        if (line[0] == '>')
        {
            nrecords++;
        }
    }
    printf("Number of records is: %d\n", nrecords);

    if (fseek(fp, pos, SEEK_SET) == -1)
    {
        perror("fseek failed in parse_fasta.");
        exit(1);
    }

    int i = 0;
    *records_ptr = malloc(nrecords * sizeof(SeqRecord));
    while ((linelen = getline(&line, &capacity, fp)) > 0)
    {
        if (line[0] == '>')
        {
            char *header = malloc(linelen + 1);
            strncpy(header, line, linelen);
            SeqRecord *p = *records_ptr + i;
            p->header = header;
            i++;
        }
    }

    if (line != NULL)
    {
        free(line);
    }

    fseek(fp, pos, SEEK_SET);
    return nrecords;
}

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
    int nrecords = parse_fasta(alignment, &records);

    // Main loop
    // Display current file
    // Read input
    // Process input
    for (int i = 0; i < nrecords; i++)
    {
        printf("Record %d\n", i);
        printf("%s\n", records[i].header);
    }

    // Clean up
    // Free memory and restore terminal options
    fclose(alignment);
    return 0;
}
