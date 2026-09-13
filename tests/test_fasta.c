#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fasta.h"
#include "formats.h"
#include "sequences.h"
#include "utils.h"

#define MODULE_NAME "test_fasta"

#define SEQ1 "This is the first sequence."
#define SEQ2 "This is the second sequence. It is a bit longer than the first."
#define SEQ3 "This is the third sequence in the array!"

SeqRecord data[] = {
    {.header = "id1 metadata1", .seq = SEQ1, .len = sizeof(SEQ1) - 1},
    {.header = "id2 metadata2", .seq = SEQ2, .len = sizeof(SEQ2) - 1},
    {.header = "id3 metadata3", .seq = SEQ3, .len = sizeof(SEQ3) - 1},
};
#define NRECORDS sizeof(data) / sizeof(SeqRecord)

SeqRecordArray record_array = {.data = data, .len = NRECORDS};

#define BUFFERLEN 1024 // Must be large enough to hold above SeqRecords
#define MAX_LEN 10     // Make small enough to ensure above records will wrap a few times

int records_equal(SeqRecordArray *record_array_1, SeqRecordArray *record_array_2)
{
    if (record_array_1->len != record_array_2->len)
        return 0;
    size_t nrecords = record_array_1->len;
    for (unsigned int i = 0; i < nrecords; i++)
    {
        SeqRecord *record1, *record2;
        record1 = record_array_1->data + i;
        record2 = record_array_2->data + i;
        if (strcmp(record1->header, record2->header) != 0)
            return 0;
        if (strcmp(record1->seq, record2->seq) != 0)
            return 0;
    }
    return 1;
}

void wrap_string_with_blanks(FILE *fp, const char *s, const unsigned int max_len)
{
    const char *t = s; // Tracks start of unwritten sequence
    unsigned int len = 0;
    for (; *s != '\0'; s++ && len++)
    {
        if (len >= max_len)
        {
            fwrite(t, sizeof(char), max_len, fp);
            fputs("\n\n", fp);
            t = s;
            len = 0;
        }
    }
    if (len > 0)
    {
        fputs(t, fp);
        fputs("\n\n", fp);
    }
}

int test_read_write(void)
{
    int retcode = TEST_SUCCESS;
    char buffer[BUFFERLEN];
    FILE *fp = fmemopen(buffer, BUFFERLEN, "rw");
    fasta_fwrite(fp, &record_array, MAX_LEN);
    fseek(fp, 0, SEEK_SET);
    SeqRecordArray new_record_array;
    int fasta_retcode = fasta_fread(fp, &new_record_array);
    TEST_ASSERT_GOTO(fasta_retcode == FORMATS_SUCCESS, cleanup, retcode);
    TEST_ASSERT_GOTO(records_equal(&record_array, &new_record_array) == 1, cleanup, retcode);
cleanup:
    if (fasta_retcode == FORMATS_SUCCESS)
        sequences_deinit_seq_record_array(&new_record_array);
    fclose(fp);
    return retcode;
}

int test_no_header(void)
{
    int retcode = TEST_SUCCESS;
    char buffer[BUFFERLEN];
    FILE *fp = fmemopen(buffer, BUFFERLEN, "rw");
    SeqRecord *record = record_array.data;
    fasta_write_sequence(fp, record->seq, MAX_LEN);
    SeqRecordArray truncated_record_array = {.data = record_array.data + 1, .len = record_array.len - 1};
    fasta_fwrite(fp, &truncated_record_array, MAX_LEN);
    fseek(fp, 0, SEEK_SET);
    SeqRecordArray new_record_array;
    int fasta_retcode = fasta_fread(fp, &new_record_array);
    TEST_ASSERT_GOTO(fasta_retcode == FORMATS_ERROR_INVALID_FORMAT, cleanup, retcode);
cleanup:
    if (fasta_retcode == 0)
        sequences_deinit_seq_record_array(&new_record_array);
    fclose(fp);
    return retcode;
}

int test_empty_file(void)
{
    int retcode = TEST_SUCCESS;
    char buffer[BUFFERLEN];
    FILE *fp = fmemopen(buffer, 1, "rw");
    fgetc(fp); // Consume the single byte of buffer
    SeqRecordArray new_record_array;
    int fasta_retcode = fasta_fread(fp, &new_record_array);
    TEST_ASSERT_GOTO(fasta_retcode == 0, cleanup, retcode);
    TEST_ASSERT_GOTO(new_record_array.len == 0, cleanup, retcode);
cleanup:
    if (fasta_retcode == 0)
        sequences_deinit_seq_record_array(&new_record_array);
    fclose(fp);
    return retcode;
}

int test_blank_lines(void)
{
    int retcode = TEST_SUCCESS;
    char buffer[BUFFERLEN];
    FILE *fp = fmemopen(buffer, BUFFERLEN, "rw");
    fputs("\n\n\n", fp);
    for (size_t i = 0; i < record_array.len; i++)
    {
        SeqRecord *record = record_array.data + i;
        fasta_write_header(fp, record->header);
        fputs("\n", fp);
        wrap_string_with_blanks(fp, record->seq, MAX_LEN);
    }
    fseek(fp, 0, SEEK_SET);
    SeqRecordArray new_record_array;
    int fasta_retcode = fasta_fread(fp, &new_record_array);
    TEST_ASSERT_GOTO(fasta_retcode == FORMATS_SUCCESS, cleanup, retcode);
    TEST_ASSERT_GOTO(new_record_array.len == record_array.len, cleanup, retcode);
    TEST_ASSERT_GOTO(records_equal(&record_array, &new_record_array) == 1, cleanup, retcode);
cleanup:
    if (fasta_retcode == FORMATS_SUCCESS)
        sequences_deinit_seq_record_array(&new_record_array);
    fclose(fp);
    return retcode;
}

int test_non_fasta(void)
{
    int retcode = TEST_SUCCESS;
    char buffer[BUFFERLEN] =
        "Here's a multiline\n"
        "file that's definitely not\n"
        "a FASTA.";
    FILE *fp = fmemopen(buffer, BUFFERLEN, "rw");
    SeqRecordArray new_record_array;
    int fasta_retcode = fasta_fread(fp, &new_record_array);
    TEST_ASSERT_GOTO(fasta_retcode == FORMATS_ERROR_INVALID_FORMAT, cleanup, retcode);
cleanup:
    if (fasta_retcode == 0)
        sequences_deinit_seq_record_array(&new_record_array);
    fclose(fp);
    return retcode;
}

int test_get_id(void)
{
    typedef struct
    {
        char *header;
        char *id;
    } IdTest;
    IdTest tests[] = {
        {"id1", "id1"},
        {"    id2", "id2"},
        {"id3    ", "id3"},
        {"    id4    ", "id4"},
        {" id5 metadata", "id5"},
        {"", ""},
        {NULL, NULL},
    };

    char *expected_id = NULL;
    char *returned_id = NULL;
    for (IdTest *test = tests; test->header != NULL; test++)
    {
        expected_id = test->id;
        returned_id = fasta_get_id(test->header);
        TEST_ASSERT(returned_id != NULL && strcmp(expected_id, returned_id) == 0);
        free(returned_id);
    }

    return TEST_SUCCESS;
}

Test tests[] = {
    {&test_read_write, "test_read_write"},
    {&test_no_header, "test_no_header"},
    {&test_empty_file, "test_empty_file"},
    {&test_blank_lines, "test_blank_lines"},
    {&test_non_fasta, "test_non_fasta"},
    {&test_get_id, "test_get_id"},
};

#define NTESTS sizeof(tests) / sizeof(Test)

int main(void)
{
    run_tests(tests, NTESTS, MODULE_NAME);
}
