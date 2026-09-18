#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fasta.h"
#include "formats.h"
#include "sequences.h"
#include "utils.h"
#include "utils_io.h"

#define MODULE_NAME "test_fasta"

int test_round_trip(void)
{
    int test_retcode = TEST_SUCCESS;
    int retcode = FORMATS_SUCCESS;
    char buffer[512];
    char header1[] = "id1 metadata1";
    char id1[] = "id1";
    char seq1[] = "This is the first sequence.";
    char header2[] = "id2 metadata2";
    char id2[] = "id2";
    char seq2[] = "This is the second sequence. It is a bit longer than the first.";
    char header3[] = "id3 metadata3";
    char id3[] = "id3";
    char seq3[] = "This is the third sequence in the array!";
    SeqRecord data[] = {
        {.header = header1, .id = id1, .seq = seq1, .len = sizeof(seq1) - 1},
        {.header = header2, .id = id2, .seq = seq2, .len = sizeof(seq2) - 1},
        {.header = header3, .id = id3, .seq = seq3, .len = sizeof(seq3) - 1},
    };
    SeqRecordArray expected_records = {.data = data, .len = sizeof(data) / sizeof(SeqRecord)};

    FILE *fp = fmemopen(buffer, sizeof(buffer), "w+b");
    TEST_ASSERT_GOTO(fp != NULL, cleanup, test_retcode);

    retcode = fasta_fwrite(fp, &expected_records, 10);
    TEST_ASSERT_GOTO(retcode == FORMATS_SUCCESS, cleanup, test_retcode);

    rewind(fp);
    SeqRecordArray returned_records = {0};
    retcode = fasta_fread(fp, &returned_records);
    TEST_ASSERT_GOTO(retcode == FORMATS_SUCCESS, cleanup, test_retcode);
    TEST_ASSERT_GOTO(records_equal(&expected_records, &returned_records) == 1, cleanup, test_retcode);

cleanup:
    if (retcode == FORMATS_SUCCESS)
        sequences_deinit_seq_record_array(&returned_records);
    if (fp != NULL)
        fclose(fp);
    return test_retcode;
}

int test_no_header(void)
{
    int test_retcode = TEST_SUCCESS;
    int retcode = FORMATS_SUCCESS;
    char buffer[] =
        "ABCD\n"
        ">id2\n"
        "12345\n";

    FILE *fp = fmemopen(buffer, sizeof(buffer) - 1, "r");
    TEST_ASSERT_GOTO(fp != NULL, cleanup, test_retcode);

    SeqRecordArray returned_records = {0};
    retcode = fasta_fread(fp, &returned_records);
    TEST_ASSERT_GOTO(retcode == FORMATS_ERROR_INVALID_FORMAT, cleanup, test_retcode);

cleanup:
    if (retcode == 0)
        sequences_deinit_seq_record_array(&returned_records);
    if (fp != NULL)
        fclose(fp);
    return test_retcode;
}

int test_empty_file(void)
{
    int test_retcode = TEST_SUCCESS;
    int retcode = FORMATS_SUCCESS;
    char buffer[1];

    FILE *fp = fmemopen(buffer, sizeof(buffer), "rw");
    TEST_ASSERT_GOTO(fp != NULL, cleanup, test_retcode);

    fgetc(fp); // Consume the single byte of buffer
    SeqRecordArray returned_records = {0};
    retcode = fasta_fread(fp, &returned_records);
    TEST_ASSERT_GOTO(retcode == 0, cleanup, test_retcode);
    TEST_ASSERT_GOTO(returned_records.len == 0, cleanup, test_retcode);

cleanup:
    if (retcode == 0)
        sequences_deinit_seq_record_array(&returned_records);
    if (fp != NULL)
        fclose(fp);
    return test_retcode;
}

int test_blank_lines(void)
{
    int test_retcode = TEST_SUCCESS;
    int retcode = FORMATS_SUCCESS;
    char buffer[] =
        "\n\n\n"
        ">id1 metadata1\n"
        "AB\n\n"
        "C\n"
        "D\n\n"
        ">id2 metadata2\n"
        "12345\n\n";
    SeqRecord data[] = {
        {.header = "id1 metadata1", .id = "id1", .seq = "ABCD", .len = 4},
        {.header = "id2 metadata2", .id = "id2", .seq = "12345", .len = 5},
    };
    SeqRecordArray expected_records = {.data = data, .len = sizeof(data) / sizeof(SeqRecord)};

    FILE *fp = fmemopen(buffer, sizeof(buffer) - 1, "r");
    TEST_ASSERT_GOTO(fp != NULL, cleanup, test_retcode);

    SeqRecordArray returned_records = {0};
    retcode = fasta_fread(fp, &returned_records);
    TEST_ASSERT_GOTO(retcode == FORMATS_SUCCESS, cleanup, test_retcode);
    TEST_ASSERT_GOTO(expected_records.len == returned_records.len, cleanup, test_retcode);
    TEST_ASSERT_GOTO(records_equal(&expected_records, &returned_records) == 1, cleanup, test_retcode);

cleanup:
    if (retcode == FORMATS_SUCCESS)
        sequences_deinit_seq_record_array(&returned_records);
    if (fp != NULL)
        fclose(fp);
    return test_retcode;
}

int test_no_eof_newline(void)
{
    int test_retcode = TEST_SUCCESS;
    int retcode = FORMATS_SUCCESS;
    char buffer[] =
        ">id1 metadata1\n"
        "ABCD\n"
        ">id2 metadata2\n"
        "12345";
    SeqRecord data[] = {
        {.header = "id1 metadata1", .id = "id1", .seq = "ABCD", .len = 4},
        {.header = "id2 metadata2", .id = "id2", .seq = "12345", .len = 5},
    };
    SeqRecordArray expected_records = {.data = data, .len = sizeof(data) / sizeof(SeqRecord)};

    FILE *fp = fmemopen(buffer, sizeof(buffer) - 1, "r");
    TEST_ASSERT_GOTO(fp != NULL, cleanup, test_retcode);

    SeqRecordArray returned_records = {0};
    retcode = fasta_fread(fp, &returned_records);
    TEST_ASSERT_GOTO(retcode == FORMATS_SUCCESS, cleanup, test_retcode);
    TEST_ASSERT_GOTO(expected_records.len == returned_records.len, cleanup, test_retcode);
    TEST_ASSERT_GOTO(records_equal(&expected_records, &returned_records) == 1, cleanup, test_retcode);

cleanup:
    if (retcode == FORMATS_SUCCESS)
        sequences_deinit_seq_record_array(&returned_records);
    if (fp != NULL)
        fclose(fp);
    return test_retcode;
}

int test_dos_line_endings(void)
{
    int test_retcode = TEST_SUCCESS;
    int retcode = FORMATS_SUCCESS;
    char buffer[] =
        ">id1 metadata1\r\n"
        "ABCD\r\n"
        ">id2 metadata2\r\n"
        "123\r\n"
        "45\r\n";
    SeqRecord data[] = {
        {.header = "id1 metadata1", .id = "id1", .seq = "ABCD", .len = 4},
        {.header = "id2 metadata2", .id = "id2", .seq = "12345", .len = 5},
    };
    SeqRecordArray expected_records = {.data = data, .len = sizeof(data) / sizeof(SeqRecord)};

    FILE *fp = fmemopen(buffer, sizeof(buffer) - 1, "r");
    TEST_ASSERT_GOTO(fp != NULL, cleanup, test_retcode);

    SeqRecordArray returned_records = {0};
    retcode = fasta_fread(fp, &returned_records);
    TEST_ASSERT_GOTO(retcode == FORMATS_SUCCESS, cleanup, test_retcode);
    TEST_ASSERT_GOTO(expected_records.len == returned_records.len, cleanup, test_retcode);
    TEST_ASSERT_GOTO(records_equal(&expected_records, &returned_records) == 1, cleanup, test_retcode);

cleanup:
    if (retcode == FORMATS_SUCCESS)
        sequences_deinit_seq_record_array(&returned_records);
    if (fp != NULL)
        fclose(fp);
    return test_retcode;
}

int test_non_fasta(void)
{
    int test_retcode = TEST_SUCCESS;
    int retcode = FORMATS_SUCCESS;
    char buffer[] =
        "Here's a multiline\n"
        "file that's definitely not\n"
        "a FASTA.";

    FILE *fp = fmemopen(buffer, sizeof(buffer), "r");
    TEST_ASSERT_GOTO(fp != NULL, cleanup, test_retcode);

    SeqRecordArray returned_records = {0};
    retcode = fasta_fread(fp, &returned_records);
    TEST_ASSERT_GOTO(retcode == FORMATS_ERROR_INVALID_FORMAT, cleanup, test_retcode);

cleanup:
    if (retcode == 0)
        sequences_deinit_seq_record_array(&returned_records);
    if (fp != NULL)
        fclose(fp);
    return test_retcode;
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
    {&test_round_trip, "test_round_trip"},
    {&test_no_header, "test_no_header"},
    {&test_empty_file, "test_empty_file"},
    {&test_blank_lines, "test_blank_lines"},
    {&test_no_eof_newline, "test_no_eof_newline"},
    {&test_dos_line_endings, "test_dos_line_endings"},
    {&test_non_fasta, "test_non_fasta"},
    {&test_get_id, "test_get_id"},
};

#define NTESTS sizeof(tests) / sizeof(Test)

int main(void)
{
    run_tests(tests, NTESTS, MODULE_NAME);
}
