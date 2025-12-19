#include <stdio.h>

#include "array.h"
#include "utils.h"

#define MODULE_NAME "test_array"

int test_init(void)
{
    int retcode = 0;
    Array array;
    int value;

    value = array_init(&array, sizeof(int));
    if (value != 0 || array.size != sizeof(int) || array.capacity == 0 || array.len != 0)
        retcode = 1;
    else
        retcode = 0;
    array_free(&array);
    return retcode;
}

int test_append_get(void)
{
    int retcode = 0;
    Array array;
    int value;

    value = array_init(&array, sizeof(int));
    if (value != 0)
    {
        retcode = 1;
        goto cleanup;
    }
    int n = 1024;
    for (int i = 0; i < n; i++)
    {
        int x = 2 * i * i - 1;
        value = array_append(&array, &x);
        if (value != 0)
        {
            retcode = 2;
            goto cleanup;
        }
    }
    for (int i = 0; i < n; i++)
    {
        int *ptr = array_get(&array, i);
        if (ptr == NULL)
        {
            retcode = 3;
            goto cleanup;
        }
        int actual_x = *ptr;
        int expected_x = 2 * i * i - 1;
        if (actual_x != expected_x)
        {
            retcode = 4;
            goto cleanup;
        }
    }
cleanup:
    array_free(&array);
    return retcode;
}

int test_extend_get(void)
{
    int retcode = 0;
    Array array_1, array_2;
    int value;

    if (array_init(&array_1, sizeof(int)) != 0 || array_init(&array_2, sizeof(int)) != 0)
    {
        retcode = 1;
        goto cleanup;
    }
    int n_1 = 1024;
    for (int i = 0; i < n_1; i++)
    {
        int x = 2 * i * i - 1;
        value = array_append(&array_1, &x);
        if (value != 0)
        {
            retcode = 2;
            goto cleanup;
        }
    }
    int n_2 = 1024;
    for (int i = 0; i < n_2; i++)
    {
        int x = i * i - 1;
        value = array_append(&array_2, &x);
        if (value != 0)
        {
            retcode = 3;
            goto cleanup;
        }
    }
    value = array_extend(&array_1, array_2.data, array_2.len);
    if (value != 0)
    {
        retcode = 4;
        goto cleanup;
    }
    for (int i = 0; i < n_1 + n_2; i++)
    {
        int *ptr = array_get(&array_1, i);
        if (ptr == NULL)
        {
            retcode = 5;
            goto cleanup;
        }
        int actual_x = *ptr;
        int expected_x;
        if (i < n_1)
            expected_x = 2 * i * i - 1;
        else if (i < n_1 + n_2)
            expected_x = (i - n_1) * (i - n_1) - 1;
        else
        {
            retcode = 6;
            goto cleanup;
        }
        if (actual_x != expected_x)
        {
            retcode = 7;
            goto cleanup;
        }
    }

cleanup:
    array_free(&array_1);
    array_free(&array_2);
    return retcode;
}

int test_get_out_of_bounds(void)
{
    int retcode = 0;
    Array array;
    int value;

    value = array_init(&array, sizeof(int));
    if (value != 0)
    {
        retcode = 1;
        goto cleanup;
    }
    size_t len = 32;
    for (size_t i = 0; i < len; i++)
    {
        value = array_append(&array, &i);
        if (value != 0)
        {
            retcode = 2;
            goto cleanup;
        }
    }
    int *ptr = array_get(&array, len - 1);
    if (ptr == NULL)
    {
        retcode = 3;
        goto cleanup;
    }
    ptr = array_get(&array, len);
    if (ptr != NULL)
    {
        retcode = 4;
        goto cleanup;
    }
cleanup:
    array_free(&array);
    return retcode;
}

int test_shrink(void)
{
    int retcode = 0;
    Array array;
    int value;

    value = array_init(&array, sizeof(int));
    if (value != 0)
    {
        retcode = 1;
        goto cleanup;
    }
    size_t capacity = array.capacity;
    array_shrink(&array);
    if (array.capacity != capacity)
    {
        retcode = 2;
        goto cleanup;
    }
    for (size_t i = 0; i < capacity + 1; i++)
        array_append(&array, &i);
    array_shrink(&array);
    if (array.capacity != array.len)
    {
        retcode = 3;
        goto cleanup;
    }
cleanup:
    array_free(&array);
    return retcode;
}

TestFunction tests[] = {
    {&test_init, "test_init"},
    {&test_append_get, "test_append"},
    {&test_extend_get, "test_extend"},
    {&test_get_out_of_bounds, "test_get_out_of_bounds"},
    {&test_shrink, "test_shrink"},
};

#define NTESTS sizeof(tests) / sizeof(TestFunction)

int main(void)
{
    run_tests(tests, NTESTS, MODULE_NAME);
}
