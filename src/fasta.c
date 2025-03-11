#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "fasta.h"
#include "sequences.h"

const int FASTA_ERROR_INVALID_FORMAT = -1;
const int FASTA_ERROR_RECORD_OVERFLOW = -2;
const int FASTA_ERROR_SEQUENCE_OVERFLOW = -3;
const int FASTA_ERROR_FILE_IO = -4;
const int FASTA_ERROR_MEMORY_ALLOCATION = -5;

int fasta_fread(FILE *fp, SeqRecord **records_ptr)
{
    // Declarations
    long pos = 0;

    void *ptr = NULL; // A generic temporary pointer for allocations

    int nrecords = 0;
    SeqRecord *new_records = NULL;
    int record_index = 0;

    char *line = NULL;
    size_t capacity = 0;
    ssize_t linelen = 0;

    ssize_t trimlen = 0;
    char *header = NULL;
    char *seq = NULL;
    size_t seqlen = 0;

    size_t bufferlen = 1;
    char *buffer = NULL;
    ptr = malloc(bufferlen);

    if (ptr == NULL)
    {
        nrecords = FASTA_ERROR_MEMORY_ALLOCATION;
        goto cleanup;
    }
    buffer = ptr;

    // Record initial position of stream
    if ((pos = ftell(fp)) == -1)
    {
        nrecords = FASTA_ERROR_FILE_IO;
        goto cleanup;
    }

    // Read until first non-empty line
    while ((linelen = getline(&line, &capacity, fp)) == 1 && line[0] == '\n')
        ;

    // Check for empty files and improper formatting
    if (linelen <= 0)
    {
        nrecords = 0;
        goto cleanup;
    }
    if (line[0] != '>')
    {
        nrecords = FASTA_ERROR_INVALID_FORMAT;
        goto cleanup;
    }

    // Count number of records
    while (linelen > 0)
    {
        if (line[0] == '>')
        {
            if (nrecords > INT_MAX - 1)
            {
                nrecords = FASTA_ERROR_RECORD_OVERFLOW;
                goto cleanup;
            }
            nrecords++;
        }
        linelen = getline(&line, &capacity, fp);
    }

    // Reset stream and allocate memory
    if (fseek(fp, pos, SEEK_SET) == -1)
    {
        nrecords = FASTA_ERROR_FILE_IO;
        goto cleanup;
    }
    ptr = malloc(nrecords * sizeof(SeqRecord));
    if (ptr == NULL)
    {
        nrecords = FASTA_ERROR_MEMORY_ALLOCATION;
        goto cleanup;
    }
    new_records = ptr;

    // Read until first non-empty line
    while ((linelen = getline(&line, &capacity, fp)) == 1 && line[0] == '\n')
        ;

    // Read records
    while (linelen > 0)
    {
        if (line[0] == '\n')
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
            nrecords = FASTA_ERROR_MEMORY_ALLOCATION;
            goto cleanup;
        }
        memcpy(header, line + 1, trimlen - 1);
        header[trimlen - 1] = '\0';

        seqlen = 0;
        while ((linelen = getline(&line, &capacity, fp)) > 0 && (line[0] != '>'))
        {
            // Trim line
            if (line[linelen - 1] == '\n')
                trimlen = linelen - 1;
            else
                trimlen = linelen;

            // Check for sequence overflow
            if (seqlen >= INT_MAX - trimlen - 1)
            {
                nrecords = FASTA_ERROR_SEQUENCE_OVERFLOW;
                goto cleanup;
            }

            // Check for buffer capacity
            while (bufferlen <= seqlen + trimlen + 1)
            {
                if (bufferlen >= INT_MAX / 2)
                {
                    nrecords = FASTA_ERROR_MEMORY_ALLOCATION;
                    goto cleanup;
                }
                ptr = realloc(buffer, 2 * bufferlen);
                if (ptr == NULL)
                {
                    nrecords = FASTA_ERROR_MEMORY_ALLOCATION;
                    goto cleanup;
                }
                buffer = ptr;
                bufferlen *= 2;
            }
            memcpy(buffer + seqlen, line, trimlen);
            seqlen += trimlen;
            buffer[seqlen] = '\0';
        }
        seq = malloc(seqlen + 1);
        if (seq == NULL)
        {
            nrecords = FASTA_ERROR_MEMORY_ALLOCATION;
            goto cleanup;
        }
        memcpy(seq, buffer, seqlen + 1);

        SeqRecord *new_record = new_records + record_index;
        new_record->header = header;
        new_record->seq = seq;
        new_record->len = seqlen;
        record_index++;
    }

    free(line);
    free(buffer);

    *records_ptr = new_records;
    return nrecords;

cleanup:
    free(line);
    free(buffer);
    free(header);
    free(seq);
    if (new_records != NULL)
    {
        header = NULL; // To guard against double frees
        seq = NULL;
        for (int i = 0; i < record_index; i++)
        {
            free(new_records[i].header);
            free(new_records[i].seq);
        }
    }
    free(new_records);
    return nrecords;
}

int fasta_read(const char *path, SeqRecord **records_ptr)
{
    FILE *fp = fopen(path, "r");
    if (fp == NULL)
        return FASTA_ERROR_FILE_IO;

    SeqRecord *ptr = NULL;
    int nrecords = fasta_fread(fp, &ptr);

    if (fclose(fp) != 0)
    {
        return FASTA_ERROR_FILE_IO;
    }

    *records_ptr = ptr;

    return nrecords;
}

int fasta_fwrite(FILE *fp, SeqRecord *records, const int nrecords, const int maxlen)
{
    for (int i = 0; i < nrecords; i++)
    {
        SeqRecord *record = records + i;
        fprintf(fp, ">%s\n", record->header);
        fasta_wrap_string(fp, record->seq, record->len, maxlen);
    }
    return 0;
}

int fasta_write(const char *path, SeqRecord *records, const int nrecords, const int maxlen)
{
    FILE *fp = fopen(path, "w");
    int code = fasta_fwrite(fp, records, nrecords, maxlen);
    fclose(fp);
    return code;
}

void fasta_wrap_string(FILE *fp, const char *s, const int len, const int maxlen)
{
    int nlines = len / maxlen;
    int j;
    for (j = 0; j < nlines; j++)
    {
        fwrite(s + j * maxlen, sizeof(char), maxlen, fp);
        fputc('\n', fp);
    }
    int nchars = len % maxlen;
    if (nchars > 0)
    {
        fwrite(s + j * maxlen, sizeof(char), nchars, fp);
        fputc('\n', fp);
    }
}