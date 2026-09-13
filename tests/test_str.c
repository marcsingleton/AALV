#define MODULE_NAME "test_str"

#include <string.h>

#include "str.h"
#include "utils.h"

int split_tester(char *s, char **expected_fields, int expected_n, const char *delim)
{
    int retcode = TEST_SUCCESS;
    char **returned_fields = NULL;
    int returned_n = str_split(&returned_fields, s, delim);
    TEST_ASSERT_GOTO(returned_n == expected_n, cleanup, retcode);
    for (int i = 0; i < expected_n; i++)
        TEST_ASSERT_GOTO(strcmp(expected_fields[i], returned_fields[i]) == 0, cleanup, retcode);
cleanup:
    if (returned_n >= 0)
        str_free_split(returned_fields, returned_n);

    return retcode;
}

int test_split_empty_input(void)
{
    char *s = "";
    char *delim = ",";
    char *expected_fields[] = {""};
    unsigned int expected_n = sizeof(expected_fields) / sizeof(char *);

    return split_tester(s, expected_fields, expected_n, delim);
}

int test_split_nonempty_fields(void)
{
    char *s = "a,1234,!@,xyz";
    char *delim = ",";
    char *expected_fields[] = {"a", "1234", "!@", "xyz"};
    unsigned int expected_n = sizeof(expected_fields) / sizeof(char *);

    return split_tester(s, expected_fields, expected_n, delim);
}

int test_split_empty_fields(void)
{
    char *s = ",a,1234,,,!@,xyz,,,";
    char *delim = ",";
    char *expected_fields[] = {"", "a", "1234", "", "", "!@", "xyz", "", "", ""};
    unsigned int expected_n = sizeof(expected_fields) / sizeof(char *);

    return split_tester(s, expected_fields, expected_n, delim);
}

int test_split_alternate_delimiter(void)
{
    char *s = "a\t1234\t!@\txyz";
    char *delim = "\t";
    char *expected_fields[] = {"a", "1234", "!@", "xyz"};
    unsigned int expected_n = sizeof(expected_fields) / sizeof(char *);

    return split_tester(s, expected_fields, expected_n, delim);
}

int test_split_wrong_expected_n(void)
{
    char *s = "a\t1234\t!@\txyz";
    char *delim = "\t";
    char *expected_fields[] = {"a", "1234", "!@"};
    unsigned int expected_n = sizeof(expected_fields) / sizeof(char *);

    if (split_tester(s, expected_fields, expected_n, delim) != 0)
        return TEST_SUCCESS;
    else
        return TEST_FAILURE;
}

int test_split_wrong_expected_fields(void)
{
    char *s = "a\t1234\t!@\txyz";
    char *delim = "\t";
    char *expected_fields[] = {"a", "1234", "!@", "xy"};
    unsigned int expected_n = sizeof(expected_fields) / sizeof(char *);

    if (split_tester(s, expected_fields, expected_n, delim) != 0)
        return TEST_SUCCESS;
    else
        return TEST_FAILURE;
}

int test_is_in_strsep(void)
{
    char *s = "a\t1234\t!@\txyz";
    char *delim = "\t";

    // True
    TEST_ASSERT(str_is_in_strsep(s, delim, "a") == 1);
    TEST_ASSERT(str_is_in_strsep(s, delim, "1234") == 1);
    TEST_ASSERT(str_is_in_strsep(s, delim, "!@") == 1);
    TEST_ASSERT(str_is_in_strsep(s, delim, "xyz") == 1);
    TEST_ASSERT(str_is_in_strsep("", delim, "") == 1); // Degenerate but consistent with empty set memberships

    // False
    TEST_ASSERT(str_is_in_strsep(s, delim, "123") == 0);
    TEST_ASSERT(str_is_in_strsep(s, delim, "789") == 0);
    TEST_ASSERT(str_is_in_strsep(s, delim, "") == 0);
    TEST_ASSERT(str_is_in_strsep("", delim, "a") == 0);

    return TEST_SUCCESS;
}

Test tests[] = {
    {&test_split_empty_input, "test_split_empty_input"},
    {&test_split_nonempty_fields, "test_split_nonempty_fields"},
    {&test_split_empty_fields, "test_split_empty_fields"},
    {&test_split_alternate_delimiter, "test_split_alternate_delimiter"},
    {&test_split_wrong_expected_n, "test_split_wrong_expected_n"},
    {&test_split_wrong_expected_fields, "test_split_wrong_expected_fields"},
    {&test_is_in_strsep, "test_is_in_strsep"},
};

#define NTESTS sizeof(tests) / sizeof(Test)

int main(void)
{
    run_tests(tests, NTESTS, MODULE_NAME);
}
