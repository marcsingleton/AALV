#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Sequences
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

// Arrays
#define INIT_CAPACITY 16
#define EXPAND_FACTOR 2

typedef struct
{
    void *data;
    size_t size;
    size_t capacity;
    size_t len;
} Array;

Array *create_array(size_t size)
{
    Array *array = malloc(sizeof(Array));
    array->data = malloc(INIT_CAPACITY * size);
    array->size = size;
    array->capacity = INIT_CAPACITY;
    array->len = 0;

    return array;
}

void append_array(void *value, Array **array_ptr)
{
    Array *array = *array_ptr;
    if (array->len < array->capacity)
    {
        void *dst = (char *)array->data + (array->len) * (array->size);
        memcpy(dst, value, array->size);
        (array->len)++;
    }
    else
    {
        printf("Array is full!\n");
    }
}

// Formats
int parse_fasta(FILE *fp, SeqRecord **records_ptr)
{
    // Record initial position of stream
    long pos;
    if ((pos = ftell(fp)) == -1)
    {
        perror("ftell failed in parse_fasta.");
        exit(1);
    }

    // Count number of records
    int nrecords = 0;
    char *line = NULL;
    size_t capacity;
    ssize_t linelen;
    while ((linelen = getline(&line, &capacity, fp)) > 0)
    {
        if (line[0] == '>')
        {
            if (INT_MAX - 1 >= nrecords)
                nrecords++;
            else
            {
                printf("Number of records in FASTA exceeds maximum (%d). Quitting...\n", INT_MAX);
                exit(1);
            }
        }
    }

    if (fseek(fp, pos, SEEK_SET) == -1)
    {
        perror("fseek failed in parse_fasta.");
        exit(1);
    }
    void *ptr = malloc(nrecords * sizeof(SeqRecord));
    if (ptr == NULL)
    {
        perror("malloc failed in parse_fasta");
        exit(1);
    }
    *records_ptr = ptr;

    // Read until first non-empty line
    size_t linespan;
    while ((linelen = getline(&line, &capacity, fp)) == 1 && line[0] == '\n')
    {
        printf("Reading until non-empty...\n");
    }
    if (line[0] != '>')
    {
        printf("First non-empty line did not start with '>' (instead %s). Quitting...\n", line);
        exit(1);
    }

    // Read records
    int i = 0;
    while (linelen > 0)
    {
        if (linespan == 1)
            continue;
        char *header = malloc(linelen + 1);
        strncpy(header, line, linelen);
        (*records_ptr + i)->header = header;

        while ((linelen = getline(&line, &capacity, fp)) > 0 && (line[0] != '>'))
        {
            printf("This is a line: %s", line);
        }
        i++;
    }

    if (line != NULL)
        free(line);
    if (fseek(fp, pos, SEEK_SET) == -1)
    {
        perror("fseek failed in parse_fasta.");
        exit(1);
    }

    return nrecords;
}

// Main
int main(int argc, char *argv[])
{
    Array *int_array = create_array(sizeof(int));
    for (int i = 0; i <= INIT_CAPACITY + 1; i++)
    {
        append_array(&i, &int_array);
    }

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
