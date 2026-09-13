#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "array.h"
#include "formats.h"
#include "linefmt.h"
#include "sequences.h"

int linefmt_fread(FILE *fp, SeqRecordArray *record_array)
{
    // Declarations
    int retcode = FORMATS_SUCCESS;

    Array new_records = {.data = NULL};

    char *line = NULL;
    size_t capacity = 0;
    ssize_t line_len = 0;

    ssize_t trim_len = 0;
    char *header = NULL;
    char *id = NULL;
    char *seq = NULL;
    size_t seq_len = 0;

    if (array_init(&new_records, sizeof(SeqRecord)) != 0)
    {
        retcode = FORMATS_ERROR_MEMORY_ALLOCATION;
        goto cleanup;
    }

    // Read records
    while ((line_len = getline(&line, &capacity, fp)) > 0)
    {
        // Trim line
        trim_len = line_len;
        while (trim_len > 0 && (line[trim_len - 1] == '\n' || line[trim_len - 1] == '\r'))
            trim_len--;
        if (trim_len == 0)
            continue;

        // Get header
        header = malloc(1);
        if (!header)
        {
            retcode = FORMATS_ERROR_MEMORY_ALLOCATION;
            goto cleanup;
        }
        header[0] = '\0';

        // Get id
        id = malloc(1);
        if (!id)
        {
            retcode = FORMATS_ERROR_MEMORY_ALLOCATION;
            goto cleanup;
        }
        id[0] = '\0';

        // Get sequence
        seq = malloc(trim_len + 1);
        if (!seq)
        {
            retcode = FORMATS_ERROR_MEMORY_ALLOCATION;
            goto cleanup;
        }
        memcpy(seq, line, trim_len);
        seq_len = trim_len;
        seq[trim_len] = '\0';

        // Create record
        SeqRecord new_record = {
            .header = header,
            .id = id,
            .seq = seq,
            .len = seq_len,
            .type = SEQ_TYPE_UNSPECIFIED,
        };

        // Check for records overflow
        if (new_records.len > SIZE_MAX - 1)
        {
            retcode = FORMATS_ERROR_RECORD_OVERFLOW;
            goto cleanup;
        }
        if (array_append(&new_records, &new_record) != 0)
        {
            retcode = FORMATS_ERROR_MEMORY_ALLOCATION;
            goto cleanup;
        }

        header = NULL;
        id = NULL;
        seq = NULL;
    }

    if (array_shrink(&new_records) != 0)
    {
        retcode = FORMATS_ERROR_MEMORY_ALLOCATION;
        goto cleanup;
    }

    record_array->data = new_records.data;
    record_array->len = new_records.len;

cleanup:
    free(line);
    free(header);
    free(id);
    free(seq);

    if (retcode != FORMATS_SUCCESS)
    {
        for (size_t i = 0; i < new_records.len; i++)
        {
            SeqRecord *record = array_get(&new_records, i);
            if (record != NULL)
            {
                free(record->header);
                free(record->id);
                free(record->seq);
            }
        }
        array_deinit(&new_records);
    }

    return retcode;
}

int linefmt_read(const char *path, SeqRecordArray *record_array)
{
    FILE *fp = fopen(path, "r");
    if (!fp)
        return FORMATS_ERROR_FILE_IO;

    int retcode = linefmt_fread(fp, record_array);
    if (retcode != FORMATS_SUCCESS)
        return retcode;

    if (fclose(fp) != 0)
        return FORMATS_ERROR_FILE_IO;

    return FORMATS_SUCCESS;
}

int linefmt_fwrite(FILE *fp, SeqRecordArray *record_array)
{
    // Check writeable
    if (fputs("", fp) == EOF)
        return FORMATS_ERROR_WRITE;

    for (size_t i = 0; i < record_array->len; i++)
    {
        SeqRecord *record = record_array->data + i;
        fputs(record->seq, fp);
        fputc('\n', fp);
    }

    return FORMATS_SUCCESS;
}

int linefmt_write(const char *path, SeqRecordArray *record_array)
{
    FILE *fp = fopen(path, "w");
    if (!fp)
        return FORMATS_ERROR_FILE_IO;

    int retcode = linefmt_fwrite(fp, record_array);
    if (retcode != FORMATS_SUCCESS)
        return retcode;

    if (fclose(fp) != 0)
        return FORMATS_ERROR_FILE_IO;

    return FORMATS_SUCCESS;
}
