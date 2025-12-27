#define MODULE_NAME "test_str"

#include <string.h>

#include "str.h"
#include "utils.h"

int split_tester(char *s, char **expected_fields, int expected_n, const char d)
{
    int retcode = 0;
    char **returned_fields = NULL;
    int returned_n = str_split(&returned_fields, s, d);
    if (returned_n != expected_n)
    {
        retcode = 1;
        goto cleanup;
    }
    for (int i = 0; i < expected_n; i++)
        if (strcmp(expected_fields[i], returned_fields[i]) != 0)
        {
            retcode = 2;
            goto cleanup;
        }
cleanup:
    if (returned_n >= 0)
        str_free_split(returned_fields, returned_n);
    return retcode;
}

int test_split_empty_input(void)
{
    char *s = "";
    char d = ',';
    char *expected_fields[] = {""};
    int expected_n = sizeof(expected_fields) / sizeof(char *);

    return split_tester(s, expected_fields, expected_n, d);
}

int test_split_nonempty_fields(void)
{
    char *s = "a,1234,!@,xyz";
    char d = ',';
    char *expected_fields[] = {"a", "1234", "!@", "xyz"};
    int expected_n = sizeof(expected_fields) / sizeof(char *);

    return split_tester(s, expected_fields, expected_n, d);
}

int test_split_empty_fields(void)
{
    char *s = ",a,1234,,,!@,xyz,,,";
    char d = ',';
    char *expected_fields[] = {"", "a", "1234", "", "", "!@", "xyz", "", "", ""};
    int expected_n = sizeof(expected_fields) / sizeof(char *);

    return split_tester(s, expected_fields, expected_n, d);
}

int test_split_alternate_delimiter(void)
{
    char *s = "a\t1234\t!@\txyz";
    char d = '\t';
    char *expected_fields[] = {"a", "1234", "!@", "xyz"};
    int expected_n = sizeof(expected_fields) / sizeof(char *);

    return split_tester(s, expected_fields, expected_n, d);
}

int test_split_wrong_expected_n(void)
{
    char *s = "a\t1234\t!@\txyz";
    char d = '\t';
    char *expected_fields[] = {"a", "1234", "!@"};
    int expected_n = sizeof(expected_fields) / sizeof(char *);

    if (split_tester(s, expected_fields, expected_n, d) != 0)
        return 0;
    else
        return 1;
}

int test_split_wrong_expected_fields(void)
{
    char *s = "a\t1234\t!@\txyz";
    char d = '\t';
    char *expected_fields[] = {"a", "1234", "!@", "xy"};
    int expected_n = sizeof(expected_fields) / sizeof(char *);

    if (split_tester(s, expected_fields, expected_n, d) != 0)
        return 0;
    else
        return 1;
}

int test_is_in_strsep(void)
{
    char *s = "a\t1234\t!@\txyz";
    char *delim = "\t";

    // True
    if (str_is_in_strsep(s, delim, "a") != 1)
        return 1;
    if (str_is_in_strsep(s, delim, "1234") != 1)
        return 1;
    if (str_is_in_strsep(s, delim, "!@") != 1)
        return 1;
    if (str_is_in_strsep(s, delim, "xyz") != 1)
        return 1;
    if (str_is_in_strsep("", delim, "") != 1) // Degenerate but consistent with empty set memberships
        return 1;
    // False
    if (str_is_in_strsep(s, delim, "123") != 0)
        return 1;
    if (str_is_in_strsep(s, delim, "789") != 0)
        return 1;
    if (str_is_in_strsep(s, delim, "") != 0)
        return 1;
    if (str_is_in_strsep("", delim, "a") != 0)
        return 1;
    return 0;
}

TestFunction tests[] = {
    {&test_split_empty_input, "test_split_empty_input"},
    {&test_split_nonempty_fields, "test_split_nonempty_fields"},
    {&test_split_empty_fields, "test_split_empty_fields"},
    {&test_split_alternate_delimiter, "test_split_alternate_delimiter"},
    {&test_split_wrong_expected_n, "test_split_wrong_expected_n"},
    {&test_split_wrong_expected_fields, "test_split_wrong_expected_fields"},
    {&test_is_in_strsep, "test_is_in_strsep"},
};

#define NTESTS sizeof(tests) / sizeof(TestFunction)

int main(void)
{
    run_tests(tests, NTESTS, MODULE_NAME);
}
