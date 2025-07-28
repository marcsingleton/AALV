#include <ctype.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>

#include "array.h"
#include "fasta.h"
#include "sequences.h"

const int FASTA_ERROR_INVALID_FORMAT = -1;
const int FASTA_ERROR_RECORD_OVERFLOW = -2;
const int FASTA_ERROR_SEQUENCE_OVERFLOW = -3;
const int FASTA_ERROR_FILE_IO = -4;
const int FASTA_ERROR_MEMORY_ALLOCATION = -5;

size_t fasta_fread(FILE *fp, SeqRecord **records_ptr)
{
    // Declarations
    size_t code;

    void *ptr = NULL; // A generic temporary pointer for allocations

    size_t nrecords = 0;
    Array new_records;
    array_init(&new_records, sizeof(SeqRecord));

    char *line = NULL;
    size_t capacity = 0;
    ssize_t linelen = 0;

    ssize_t trimlen = 0;
    char *header = NULL;
    char *seq = NULL;
    char *id = NULL;
    size_t seqlen = 0;

    size_t bufferlen = 256;
    char *buffer = NULL;
    ptr = malloc(bufferlen);

    if (ptr == NULL)
    {
        code = FASTA_ERROR_MEMORY_ALLOCATION;
        goto error;
    }
    buffer = ptr;

    // Read until first non-empty line
    while ((linelen = getline(&line, &capacity, fp)) == 1 && line[0] == '\n')
        ;

    // Check for empty files and improper formatting
    if (linelen <= 0)
    {
        code = 0;
        goto error;
    }
    if (line[0] != '>')
    {
        code = FASTA_ERROR_INVALID_FORMAT;
        goto error;
    }

    // Read records
    while (linelen > 0)
    {
        // Get header
        if (line[0] == '\n')
        {
            linelen = getline(&line, &capacity, fp);
            continue;
        }

        trimlen = linelen;
        while (line[trimlen - 1] == '\n' || line[trimlen - 1] == '\r')
            trimlen--;

        header = malloc(trimlen); // +1 for null; -1 for excluding >
        if (header == NULL)
        {
            code = FASTA_ERROR_MEMORY_ALLOCATION;
            goto error;
        }
        memcpy(header, line + 1, trimlen - 1);
        header[trimlen - 1] = '\0';

        // Get id
        char *id = fasta_get_id(header);
        if (id == NULL)
        {
            code = FASTA_ERROR_MEMORY_ALLOCATION;
            goto error;
        }

        // Get seq
        seqlen = 0;
        while ((linelen = getline(&line, &capacity, fp)) > 0 && (line[0] != '>'))
        {
            // Trim line
            trimlen = linelen;
            while (line[trimlen - 1] == '\n' || line[trimlen - 1] == '\r')
                trimlen--;

            // Check for sequence overflow
            if (seqlen >= SIZE_MAX - trimlen - 1)
            {
                code = FASTA_ERROR_SEQUENCE_OVERFLOW;
                goto error;
            }

            // Check for buffer capacity
            while (bufferlen <= seqlen + trimlen + 1)
            {
                if (bufferlen >= SIZE_MAX / 2)
                {
                    code = FASTA_ERROR_MEMORY_ALLOCATION;
                    goto error;
                }
                ptr = realloc(buffer, 2 * bufferlen);
                if (ptr == NULL)
                {
                    code = FASTA_ERROR_MEMORY_ALLOCATION;
                    goto error;
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
            code = FASTA_ERROR_MEMORY_ALLOCATION;
            goto error;
        }
        memcpy(seq, buffer, seqlen + 1);

        SeqRecord new_record = {
            .header = header,
            .id = id,
            .seq = seq,
            .len = seqlen,
            .type = SEQ_TYPE_UNSPECIFIED,
        };
        if (array_append(&new_records, &new_record) != 0)
        {
            code = FASTA_ERROR_RECORD_OVERFLOW;
            goto error;
        };
    }

    free(line);
    free(buffer);

    if (array_shrink(&new_records) != 0)
    {
        code = FASTA_ERROR_MEMORY_ALLOCATION;
        goto error;
    }
    *records_ptr = new_records.data;
    nrecords = new_records.len;
    return nrecords;

error:
    free(line);
    free(buffer);
    free(header);
    free(id);
    free(seq);

    header = NULL; // To guard against double frees
    id = NULL;
    seq = NULL;
    for (size_t i = 0; i < new_records.len; i++)
    {
        SeqRecord *new_record = array_get(&new_records, i);
        free(new_record->header);
        free(new_record->id);
        free(new_record->seq);
    }

    array_free(&new_records);
    return code;
}

size_t fasta_read(const char *path, SeqRecord **records_ptr)
{
    FILE *fp = fopen(path, "r");
    if (fp == NULL)
        return FASTA_ERROR_FILE_IO;

    SeqRecord *ptr = NULL;
    size_t nrecords = fasta_fread(fp, &ptr);

    if (fclose(fp) != 0)
        return FASTA_ERROR_FILE_IO;

    *records_ptr = ptr;

    return nrecords;
}

int fasta_fwrite(FILE *fp, SeqRecord *records, const size_t nrecords, const int maxlen)
{
    for (size_t i = 0; i < nrecords; i++)
    {
        SeqRecord *record = records + i;
        fprintf(fp, ">%s\n", record->header);
        fasta_wrap_string(fp, record->seq, record->len, maxlen);
    }
    return 0;
}

int fasta_write(const char *path, SeqRecord *records, const size_t nrecords, const int maxlen)
{
    FILE *fp = fopen(path, "w");
    int code = fasta_fwrite(fp, records, nrecords, maxlen);
    fclose(fp);
    return code;
}

void fasta_wrap_string(FILE *fp, const char *s, const size_t len, const int maxlen)
{
    size_t nlines = len / maxlen;
    size_t j;
    for (j = 0; j < nlines; j++)
    {
        fwrite(s + j * maxlen, sizeof(char), maxlen, fp);
        fputc('\n', fp);
    }
    size_t nchars = len % maxlen;
    if (nchars > 0)
    {
        fwrite(s + j * maxlen, sizeof(char), nchars, fp);
        fputc('\n', fp);
    }
}

char *fasta_get_id(const char *header)
{
    size_t start = SIZE_MAX, stop = SIZE_MAX;
    size_t i = 0;

    // Check for start and stop
    // A string in memory without the null terminator will always be less than SIZE_MAX, so no need to check
    char c;
    while ((c = header[i]) != '\0')
    {
        if (start == SIZE_MAX && !isspace(c))
            start = i;
        else if (start != SIZE_MAX && isspace(c))
        {
            stop = i;
            break;
        }
        i++;
    }

    size_t len;
    if (start == SIZE_MAX) // No start
        len = 0;
    else if (stop == SIZE_MAX) // Start but no stop
        len = i - start;
    else // Start and stop
        len = stop - start;

    // Allocate memory
    char *id = malloc(len + 1);
    if (id == NULL)
        return id;
    strncpy(id, header + start, len);
    *(id + len) = '\0';
    return id;
}
