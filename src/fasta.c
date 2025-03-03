#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "fasta.h"
#include "seqrecord.h"

const int FASTA_ERROR_INVALID_FORMAT = -1;
const int FASTA_ERROR_RECORD_OVERFLOW = -2;
const int FASTA_ERROR_SEQUENCE_OVERFLOW = -3;
const int FASTA_ERROR_FILE_IO = -4;
const int FASTA_ERROR_MEMORY_ALLOCATION = -5;

void fasta_parse_free(char *line, char *header, char *seq)
{
    if (line != NULL)
        free(line);
    if (header != NULL)
        free(header);
    if (seq != NULL)
        free(seq);
}

int fasta_parse(FILE *fp, SeqRecord **records_ptr)
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
        fasta_parse_free(line, header, seq);
        return FASTA_ERROR_FILE_IO;
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
                fasta_parse_free(line, header, seq);
                return FASTA_ERROR_RECORD_OVERFLOW;
            }
        }
    }
    if (nrecords == 0)
    {
        fasta_parse_free(line, header, seq);
        return nrecords;
    }

    // Reset stream and allocate memory
    if (fseek(fp, pos, SEEK_SET) == -1)
    {
        fasta_parse_free(line, header, seq);
        return FASTA_ERROR_FILE_IO;
    }
    void *ptr = malloc(nrecords * sizeof(SeqRecord));
    if (ptr == NULL)
    {
        fasta_parse_free(line, header, seq);
        return FASTA_ERROR_MEMORY_ALLOCATION;
    }
    *records_ptr = ptr;

    // Read until first non-empty line
    while ((linelen = getline(&line, &capacity, fp)) == 1 && line[0] == '\n')
    {
    }

    if (line[0] != '>')
    {
        fasta_parse_free(line, header, seq);
        return FASTA_ERROR_INVALID_FORMAT;
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
            fasta_parse_free(line, header, seq);
            return FASTA_ERROR_MEMORY_ALLOCATION;
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
                fasta_parse_free(line, header, seq);
                return FASTA_ERROR_SEQUENCE_OVERFLOW;
            }
            ptr = realloc(seq, seqlen + trimlen + 1);
            if (ptr == NULL)
            {
                fasta_parse_free(line, header, seq);
                return FASTA_ERROR_MEMORY_ALLOCATION;
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

    free(line);

    return nrecords;
}

int fasta_read(const char *path, SeqRecord **records_ptr)
{
    FILE *fp = fopen(path, "r");
    if (fp == NULL)
        return FASTA_ERROR_FILE_IO;

    SeqRecord *ptr = malloc(sizeof(SeqRecord));
    if (ptr == NULL)
        return FASTA_ERROR_MEMORY_ALLOCATION;

    int nrecords = fasta_parse(fp, &ptr);

    if (fclose(fp) != 0)
    {
        free(ptr);
        return FASTA_ERROR_FILE_IO;
    }

    *records_ptr = ptr;

    return nrecords;
}
