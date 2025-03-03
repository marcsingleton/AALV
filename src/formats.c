#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "formats.h"

void parse_fasta_free(char *line, char *header, char *seq)
{
    if (line != NULL)
        free(line);
    if (header != NULL)
        free(header);
    if (seq != NULL)
        free(seq);
}

int parse_fasta(FILE *fp, SeqRecord **records_ptr)
{
    // Declarations
    long pos;

    int nrecords = 0;

    char *line = NULL;
    size_t capacity;
    ssize_t linelen;

    ssize_t trimlen;
    char *header = NULL;
    char *seq = NULL;
    size_t seqlen = 0;

    // Record initial position of stream
    if ((pos = ftell(fp)) == -1)
    {
        parse_fasta_free(line, header, seq);
        return -3;
    }

    // Count number of records
    while ((linelen = getline(&line, &capacity, fp)) > 0)
    {
        if (line[0] == '>')
        {
            if (INT_MAX - 1 >= nrecords)
                nrecords++;
            else
            {
                parse_fasta_free(line, header, seq);
                return -1;
            }
        }
    }
    if (nrecords == 0)
    {
        parse_fasta_free(line, header, seq);
        return nrecords;
    }

    // Reset stream and allocate memory
    if (fseek(fp, pos, SEEK_SET) == -1)
    {
        parse_fasta_free(line, header, seq);
        return -3;
    }
    void *ptr = malloc(nrecords * sizeof(SeqRecord));
    if (ptr == NULL)
    {
        parse_fasta_free(line, header, seq);
        return -4;
    }
    *records_ptr = ptr;

    // Read until first non-empty line
    while ((linelen = getline(&line, &capacity, fp)) == 1 && line[0] == '\n')
        printf("Reading until non-empty...\n");
    if (line[0] != '>')
    {
        parse_fasta_free(line, header, seq);
        return -2;
    }

    // Read records
    int i = 0;
    while (linelen > 0)
    {
        if (linelen == 1)
        {
            linelen = getline(&line, &capacity, fp);
            continue;
        }

        if (line[linelen - 1] == '\n')
            trimlen = linelen - 1;
        else
            trimlen = linelen;

        header = malloc(trimlen); // +1 for null; -1 for excluding >
        if (header == NULL)
        {
            parse_fasta_free(line, header, seq);
            return -4;
        }
        memcpy(header, line + 1, trimlen - 1);
        header[trimlen - 1] = '\0';

        seq = NULL;
        seqlen = 0;
        while ((linelen = getline(&line, &capacity, fp)) > 0 && (line[0] != '>'))
        {
            if (line[linelen - 1] == '\n')
                trimlen = linelen - 1;
            else
                trimlen = linelen;

            if (seqlen >= INT_MAX - trimlen - 1)
            {
                parse_fasta_free(line, header, seq);
                return -5;
            }
            ptr = realloc(seq, seqlen + trimlen + 1);
            if (ptr == NULL)
            {
                parse_fasta_free(line, header, seq);
                return -4;
            }

            seq = ptr;
            memcpy(seq + seqlen, line, trimlen);
            seqlen += trimlen;
            seq[seqlen] = '\0';
        }
        (*records_ptr + i)->header = header;
        (*records_ptr + i)->seq = seq;
        i++;
    }

    if (fseek(fp, pos, SEEK_SET) == -1)
    {
        parse_fasta_free(line, header, seq);
        return -3;
    }

    free(line);

    return nrecords;
}
