#include <stdio.h>

#include "array.h"
#include "utils.h"

#define MODULE_NAME "test_array"

int test_init(void)
{
    int code = 0;
    Array array;
    int value;

    value = array_init(&array, sizeof(int));
    if (value != 0 || array.size != sizeof(int) || array.capacity == 0 || array.len != 0)
        code = 1;
    else
        code = 0;
    array_free(&array);
    return code;
}

int test_append_get(void)
{
    int code = 0;
    Array array;
    int value;

    value = array_init(&array, sizeof(int));
    if (value != 0)
    {
        code = 1;
        goto cleanup;
    }
    int n = 1024;
    for (int i = 0; i < n; i++)
    {
        value = array_append(&array, &i);
        if (value != 0)
        {
            code = 2;
            goto cleanup;
        }
    }
    for (int i = 0; i < 1024; i++)
    {
        int *ptr = array_get(&array, i);
        if (ptr == NULL)
        {
            code = 3;
            goto cleanup;
        }
        int x = *ptr;
        if (x != i)
        {
            code = 4;
            goto cleanup;
        }
    }
cleanup:
    array_free(&array);
    return code;
}

int test_get_out_of_bounds(void)
{
    int code = 0;
    Array array;
    int value;
    value = array_init(&array, sizeof(int));

    size_t len = 32;
    if (value != 0)
    {
        code = 1;
        goto cleanup;
    }
    for (size_t i = 0; i < len; i++)
    {
        value = array_append(&array, &i);
        if (value != 0)
        {
            code = 2;
            goto cleanup;
        }
    }
    int *ptr = array_get(&array, len - 1);
    if (ptr == NULL)
    {
        code = 3;
        goto cleanup;
    }
    ptr = array_get(&array, len);
    if (ptr != NULL)
    {
        code = 4;
        goto cleanup;
    }
cleanup:
    array_free(&array);
    return code;
}

TestFunction tests[] = {
    {&test_init, "test_init"},
    {&test_append_get, "test_append"},
    {&test_get_out_of_bounds, "test_get_out_of_bounds"},
};

#define NTESTS sizeof(tests) / sizeof(TestFunction)

int main(void)
{
    run_tests(tests, NTESTS, MODULE_NAME);
}
