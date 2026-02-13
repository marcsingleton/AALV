#include <ctype.h>
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "array.h"
#include "fasta.h"
#include "sequences.h"

int fasta_fread(FILE *fp, SeqRecordArray *record_array)
{
    // Declarations
    int retcode = FASTA_ERROR_SUCCESS;

    void *ptr = NULL; // A generic temporary pointer for allocations

    Array new_records;
    array_init(&new_records, sizeof(SeqRecord));

    char *line = NULL;
    size_t capacity = 0;
    ssize_t line_len = 0;

    ssize_t trim_len = 0;
    char *header = NULL;
    char *seq = NULL;
    char *id = NULL;
    size_t seq_len = 0;

    size_t buffer_len = 256;
    char *buffer = malloc(buffer_len);
    if (!buffer)
    {
        retcode = FASTA_ERROR_MEMORY_ALLOCATION;
        goto cleanup;
    }

    // Read until first non-empty line
    while ((line_len = getline(&line, &capacity, fp)) == 1 && line[0] == '\n')
        ;

    // Check for empty files and improper formatting
    if (line_len <= 0)
    {
        record_array->data = NULL;
        record_array->len = 0;
        goto cleanup;
    }
    if (line[0] != '>')
    {
        retcode = FASTA_ERROR_INVALID_FORMAT;
        goto cleanup;
    }

    // Read records
    while (line_len > 0)
    {
        // Get header
        if (line[0] == '\n')
        {
            line_len = getline(&line, &capacity, fp);
            continue;
        }

        trim_len = line_len;
        while (line[trim_len - 1] == '\n' || line[trim_len - 1] == '\r')
            trim_len--;

        header = malloc(trim_len); // +1 for null; -1 for excluding >
        if (!header)
        {
            retcode = FASTA_ERROR_MEMORY_ALLOCATION;
            goto cleanup;
        }
        memcpy(header, line + 1, trim_len - 1);
        header[trim_len - 1] = '\0';

        // Get id
        id = fasta_get_id(header);
        if (!id)
        {
            retcode = FASTA_ERROR_MEMORY_ALLOCATION;
            goto cleanup;
        }

        // Get seq
        seq_len = 0;
        while ((line_len = getline(&line, &capacity, fp)) > 0 && (line[0] != '>'))
        {
            // Trim line
            trim_len = line_len;
            while (trim_len > 0 && (line[trim_len - 1] == '\n' || line[trim_len - 1] == '\r'))
                trim_len--;

            // Check for sequence overflow
            if (seq_len > SIZE_MAX - trim_len - 1)
            {
                retcode = FASTA_ERROR_SEQUENCE_OVERFLOW;
                goto cleanup;
            }

            // Check for buffer capacity
            while (buffer_len <= seq_len + trim_len + 1)
            {
                if (buffer_len > SIZE_MAX / 2)
                {
                    retcode = FASTA_ERROR_MEMORY_ALLOCATION;
                    goto cleanup;
                }
                ptr = realloc(buffer, 2 * buffer_len);
                if (!ptr)
                {
                    retcode = FASTA_ERROR_MEMORY_ALLOCATION;
                    goto cleanup;
                }
                buffer = ptr;
                buffer_len *= 2;
            }
            memcpy(buffer + seq_len, line, trim_len);
            seq_len += trim_len;
            buffer[seq_len] = '\0';
        }
        seq = malloc(seq_len + 1);
        if (!seq)
        {
            retcode = FASTA_ERROR_MEMORY_ALLOCATION;
            goto cleanup;
        }
        memcpy(seq, buffer, seq_len + 1);

        SeqRecord new_record = {
            .header = header,
            .id = id,
            .seq = seq,
            .len = seq_len,
            .type = SEQ_TYPE_UNSPECIFIED,
        };
        if (new_records.len > SIZE_MAX - 1) // Ensures fit into return type
        {
            retcode = FASTA_ERROR_RECORD_OVERFLOW;
            goto cleanup;
        }
        if (array_append(&new_records, &new_record) != 0)
        {
            retcode = FASTA_ERROR_RECORD_OVERFLOW;
            goto cleanup;
        }
        header = NULL;
        id = NULL;
        seq = NULL;
    }

    if (array_shrink(&new_records) != 0)
    {
        retcode = FASTA_ERROR_MEMORY_ALLOCATION;
        goto cleanup;
    }

    record_array->data = new_records.data;
    record_array->len = new_records.len;

cleanup:
    free(line);
    free(buffer);
    free(header);
    free(id);
    free(seq);

    if (retcode != FASTA_ERROR_SUCCESS)
    {
        for (size_t i = 0; i < new_records.len; i++)
        {
            SeqRecord *new_record = array_get(&new_records, i);
            free(new_record->header);
            free(new_record->id);
            free(new_record->seq);
        }
        array_deinit(&new_records);
    }
    return retcode;
}

int fasta_read(const char *path, SeqRecordArray *record_array)
{
    FILE *fp = fopen(path, "r");
    if (!fp)
        return FASTA_ERROR_FILE_IO;

    int retcode = fasta_fread(fp, record_array);

    if (fclose(fp) != 0)
        return FASTA_ERROR_FILE_IO;

    return retcode;
}

int fasta_fwrite(FILE *fp, SeqRecordArray *record_array, const int max_len)
{
    for (size_t i = 0; i < record_array->len; i++)
    {
        SeqRecord *record = record_array->data + i;
        fprintf(fp, ">%s\n", record->header);
        fasta_wrap_string(fp, record->seq, record->len, max_len);
    }
    return 0;
}

int fasta_write(const char *path, SeqRecordArray *record_array, const int max_len)
{
    FILE *fp = fopen(path, "w");
    if (!fp)
        return FASTA_ERROR_FILE_IO;

    int retcode = fasta_fwrite(fp, record_array, max_len);

    if (fclose(fp) != 0)
        return FASTA_ERROR_FILE_IO;

    return retcode;
}

void fasta_wrap_string(FILE *fp, const char *s, const size_t len, const int max_len)
{
    size_t nlines = len / max_len;
    size_t j;
    for (j = 0; j < nlines; j++)
    {
        fwrite(s + j * max_len, sizeof(char), max_len, fp);
        fputc('\n', fp);
    }
    size_t nchars = len % max_len;
    if (nchars > 0)
    {
        fwrite(s + j * max_len, sizeof(char), nchars, fp);
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
    if (!id)
        return id;
    strncpy(id, header + start, len);
    *(id + len) = '\0';
    return id;
}
