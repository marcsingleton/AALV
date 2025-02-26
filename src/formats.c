#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "formats.h"

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
        printf("Reading until non-empty...\n");
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
        {
            linelen = getline(&line, &capacity, fp);
            continue;
        }

        char *header = malloc(linelen);
        if (header == NULL)
        {
            perror("malloc failed in parse_fasta");
            exit(1);
        }
        memcpy(header, line + 1, linelen - 1);
        header[linelen - 2] = '\0';

        char *seq = NULL;
        size_t seqlen = 0;
        while ((linelen = getline(&line, &capacity, fp)) > 0 && (line[0] != '>'))
        {
            ptr = realloc(seq, seqlen + linelen);
            if (ptr == NULL)
            {
                perror("realloc failed in parse_fasta");
                exit(1);
            }
            seq = ptr;
            memcpy(seq + seqlen, line, linelen);
            seqlen += linelen - 1;
            seq[seqlen] = '\0';
        }
        (*records_ptr + i)->header = header;
        (*records_ptr + i)->seq = seq;
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
