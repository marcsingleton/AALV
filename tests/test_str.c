#define MODULE_NAME "test_str"

#include <string.h>

#include "str.h"
#include "utils.h"

int split_tester(char *s, char **expected_fields, int expected_n, const char d)
{
    int code = 0;
    char **returned_fields = NULL;
    int returned_n = str_split(&returned_fields, s, d);
    if (returned_n != expected_n)
    {
        code = 1;
        goto cleanup;
    }
    for (int i = 0; i < expected_n; i++)
        if (strcmp(expected_fields[i], returned_fields[i]) != 0)
        {
            code = 2;
            goto cleanup;
        }
cleanup:
    if (returned_n >= 0)
        str_free_split(returned_fields, returned_n);
    return code;
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

TestFunction tests[] = {
    {&test_split_empty_input, "test_split_empty_input"},
    {&test_split_nonempty_fields, "test_split_nonempty_fields"},
    {&test_split_empty_fields, "test_split_empty_fields"},
    {&test_split_alternate_delimiter, "test_split_alternate_delimiter"},
    {&test_split_wrong_expected_n, "test_split_wrong_expected_n"},
    {&test_split_wrong_expected_fields, "test_split_wrong_expected_fields"},
};

#define NTESTS sizeof(tests) / sizeof(TestFunction)

int main(void)
{
    run_tests(tests, NTESTS, MODULE_NAME);
}
