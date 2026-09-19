#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "formats.h"
#include "linefmt.h"
#include "sequences.h"
#include "utils.h"
#include "utils_io.h"

#define MODULE_NAME "test_linefmt"

int test_round_trip(void)
{
    int test_retcode = TEST_SUCCESS;
    int retcode = FORMATS_SUCCESS;
    char buffer[512];
    SeqRecord data[] = {
        {.header = "", .id = "", .seq = "ABCD", .len = 4},
        {.header = "", .id = "", .seq = "12345", .len = 5},
        {.header = "", .id = "", .seq = "FGHI", .len = 4},
    };
    SeqRecordArray exp_records = {.data = data, .len = sizeof(data) / sizeof(SeqRecord)};

    FILE *fp = fmemopen(buffer, sizeof(buffer), "w+");
    TEST_ASSERT_GOTO(fp != NULL, cleanup, test_retcode);

    retcode = linefmt_fwrite(fp, &exp_records);
    TEST_ASSERT_GOTO(retcode == FORMATS_SUCCESS, cleanup, test_retcode);

    rewind(fp);
    SeqRecordArray ret_records = {0};
    retcode = linefmt_fread(fp, &ret_records);
    TEST_ASSERT_GOTO(retcode == FORMATS_SUCCESS, cleanup, test_retcode);
    TEST_ASSERT_GOTO(records_equal(&exp_records, &ret_records) == 1, cleanup, test_retcode);

cleanup:
    if (retcode == FORMATS_SUCCESS)
        sequences_deinit_seq_record_array(&ret_records);
    if (fp != NULL)
        fclose(fp);
    return test_retcode;
}

int test_skip_empty_lines(void)
{
    int test_retcode = TEST_SUCCESS;
    int retcode = FORMATS_SUCCESS;
    char buffer[] =
        "ABCD\n\n"
        "12345\n\n\n"
        "FGHI\n";
    SeqRecord data[] = {
        {.header = "", .id = "", .seq = "ABCD", .len = 4},
        {.header = "", .id = "", .seq = "12345", .len = 5},
        {.header = "", .id = "", .seq = "FGHI", .len = 4},
    };
    SeqRecordArray exp_records = {.data = data, .len = sizeof(data) / sizeof(SeqRecord)};

    FILE *fp = fmemopen(buffer, sizeof(buffer) - 1, "r");
    TEST_ASSERT_GOTO(fp != NULL, cleanup, test_retcode);

    SeqRecordArray ret_records = {0};
    retcode = linefmt_fread(fp, &ret_records);
    TEST_ASSERT_GOTO(retcode == FORMATS_SUCCESS, cleanup, test_retcode);
    TEST_ASSERT_GOTO(exp_records.len == ret_records.len, cleanup, test_retcode);
    TEST_ASSERT_GOTO(records_equal(&exp_records, &ret_records) == 1, cleanup, test_retcode);

cleanup:
    if (retcode == FORMATS_SUCCESS)
        sequences_deinit_seq_record_array(&ret_records);
    if (fp != NULL)
        fclose(fp);
    return test_retcode;
}

int test_dos_line_endings(void)
{
    int test_retcode = TEST_SUCCESS;
    int retcode = FORMATS_SUCCESS;
    char buffer[] =
        "ABCD\r\n"
        "12345\r\n"
        "FGHI\r\n";
    SeqRecord data[] = {
        {.header = "", .id = "", .seq = "ABCD", .len = 4},
        {.header = "", .id = "", .seq = "12345", .len = 5},
        {.header = "", .id = "", .seq = "FGHI", .len = 4},
    };
    SeqRecordArray exp_records = {.data = data, .len = sizeof(data) / sizeof(SeqRecord)};

    FILE *fp = fmemopen(buffer, sizeof(buffer) - 1, "r");
    TEST_ASSERT_GOTO(fp != NULL, cleanup, test_retcode);

    SeqRecordArray ret_records = {0};
    retcode = linefmt_fread(fp, &ret_records);
    TEST_ASSERT_GOTO(retcode == FORMATS_SUCCESS, cleanup, test_retcode);
    TEST_ASSERT_GOTO(exp_records.len == ret_records.len, cleanup, test_retcode);
    TEST_ASSERT_GOTO(records_equal(&exp_records, &ret_records) == 1, cleanup, test_retcode);

cleanup:
    if (retcode == FORMATS_SUCCESS)
        sequences_deinit_seq_record_array(&ret_records);
    if (fp != NULL)
        fclose(fp);
    return test_retcode;
}

int test_empty_file(void)
{
    int test_retcode = TEST_SUCCESS;
    int retcode = FORMATS_SUCCESS;
    char buffer[1];

    FILE *fp = fmemopen(buffer, sizeof(buffer), "w+");
    TEST_ASSERT_GOTO(fp != NULL, cleanup, test_retcode);

    rewind(fp);
    SeqRecordArray ret_records = {0};
    retcode = linefmt_fread(fp, &ret_records);
    TEST_ASSERT_GOTO(retcode == FORMATS_SUCCESS, cleanup, test_retcode);
    TEST_ASSERT_GOTO(ret_records.len == 0, cleanup, test_retcode);

cleanup:
    if (retcode == FORMATS_SUCCESS && ret_records.data != NULL)
        sequences_deinit_seq_record_array(&ret_records);
    if (fp != NULL)
        fclose(fp);
    return test_retcode;
}

int main(void)
{
    Test tests[] = {
        {test_round_trip, "test_round_trip"},
        {test_skip_empty_lines, "test_skip_empty_lines"},
        {test_dos_line_endings, "test_dos_line_endings"},
        {test_empty_file, "test_empty_file"},
    };

    run_tests(tests, sizeof(tests) / sizeof(Test), MODULE_NAME);
    return 0;
}
