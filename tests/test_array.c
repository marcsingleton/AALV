#include <stdio.h>

#include "array.h"
#include "utils.h"

#define MODULE_NAME "test_array"

int test_init(void)
{
    int retcode = TEST_SUCCESS;
    Array array = {0};
    int value;

    // NULL array
    value = array_init(NULL, sizeof(int));
    TEST_ASSERT_GOTO(value != 0, cleanup, retcode);

    // 0 size
    value = array_init(&array, 0);
    TEST_ASSERT_GOTO(value != 0, cleanup, retcode);

    // Success
    value = array_init(&array, sizeof(int));
    int condition = value == 0 && array.size == sizeof(int) && array.capacity != 0 && array.len == 0;
    TEST_ASSERT_GOTO(condition, cleanup, retcode);

cleanup:
    array_deinit(&array);
    return retcode;
}

int test_append_null_array(void)
{
    int retcode = TEST_SUCCESS;
    int value;

    int x = 0;
    value = array_append(NULL, &x);
    TEST_ASSERT_GOTO(value != 0, cleanup, retcode);

cleanup:
    return retcode;
}

int test_append_null_value(void)
{
    int retcode = TEST_SUCCESS;
    Array array = {0};
    int value;

    value = array_init(&array, sizeof(int));
    TEST_ASSERT_GOTO(value == 0, cleanup, retcode);

    value = array_append(&array, NULL);
    TEST_ASSERT_GOTO(value != 0, cleanup, retcode);

cleanup:
    array_deinit(&array);
    return retcode;
}

int test_extend_null_array(void)
{
    int retcode = TEST_SUCCESS;
    int value;

    int x[] = {0, 1, 2};
    value = array_extend(NULL, &x, sizeof(x) / sizeof(int));
    TEST_ASSERT_GOTO(value != 0, cleanup, retcode);

cleanup:
    return retcode;
}

int test_extend_null_values(void)
{
    int retcode = TEST_SUCCESS;
    Array array = {0};
    int value;

    value = array_init(&array, sizeof(int));
    TEST_ASSERT_GOTO(value == 0, cleanup, retcode);

    value = array_extend(&array, NULL, 0);
    TEST_ASSERT_GOTO(value != 0, cleanup, retcode);

cleanup:
    array_deinit(&array);
    return retcode;
}

int test_get_null(void)
{
    int retcode = TEST_SUCCESS;

    // NULL array
    int *ptr = array_get(NULL, 0);
    TEST_ASSERT_GOTO(ptr == NULL, cleanup, retcode);

cleanup:
    return retcode;
}

int test_append_get(void)
{
    int retcode = TEST_SUCCESS;
    Array array = {0};
    int value;

    // Initialize
    value = array_init(&array, sizeof(int));
    TEST_ASSERT_GOTO(value == 0, cleanup, retcode);

    // Append
    int n = 1024;
    for (int i = 0; i < n; i++)
    {
        int x = 2 * i * i - 1;
        value = array_append(&array, &x);
        TEST_ASSERT_GOTO(value == 0, cleanup, retcode);
    }

    // Get
    for (int i = 0; i < n; i++)
    {
        int *ptr = array_get(&array, i);
        TEST_ASSERT_GOTO(ptr != NULL, cleanup, retcode);
        int actual_x = *ptr;
        int expected_x = 2 * i * i - 1;
        TEST_ASSERT_GOTO(actual_x == expected_x, cleanup, retcode);
    }

cleanup:
    array_deinit(&array);
    return retcode;
}

int test_extend_get(void)
{
    int retcode = TEST_SUCCESS;
    Array array1 = {0};
    Array array2 = {0};
    int value;

    // Initialize
    int condition = array_init(&array1, sizeof(int)) == 0 && array_init(&array2, sizeof(int)) == 0;
    TEST_ASSERT_GOTO(condition, cleanup, retcode);

    // Append
    int n_1 = 1024;
    for (int i = 0; i < n_1; i++)
    {
        int x = 2 * i * i - 1;
        value = array_append(&array1, &x);
        TEST_ASSERT_GOTO(value == 0, cleanup, retcode);
    }
    int n_2 = 1024;
    for (int i = 0; i < n_2; i++)
    {
        int x = i * i - 1;
        value = array_append(&array2, &x);
        TEST_ASSERT_GOTO(value == 0, cleanup, retcode);
    }

    // Extend
    value = array_extend(&array1, array2.data, array2.len);
    TEST_ASSERT_GOTO(value == 0, cleanup, retcode);
    for (int i = 0; i < n_1 + n_2; i++)
    {
        int *ptr = array_get(&array1, i);
        TEST_ASSERT_GOTO(ptr != NULL, cleanup, retcode);
        int actual_x = *ptr;
        int expected_x;
        if (i < n_1)
            expected_x = 2 * i * i - 1;
        else if (i < n_1 + n_2)
            expected_x = (i - n_1) * (i - n_1) - 1;
        else
        {
            retcode = -6;
            goto cleanup;
        }
        TEST_ASSERT_GOTO(actual_x == expected_x, cleanup, retcode);
    }

cleanup:
    array_deinit(&array1);
    array_deinit(&array2);
    return retcode;
}

int test_append_pop(void)
{
    int retcode = TEST_SUCCESS;
    Array array = {0};
    int value;

    // Initialize
    value = array_init(&array, sizeof(int));
    TEST_ASSERT_GOTO(value == 0, cleanup, retcode);

    // Append
    int n = 1024;
    for (int i = 0; i < n; i++)
    {
        int x = 2 * i * i - 1;
        value = array_append(&array, &x);
        TEST_ASSERT_GOTO(value == 0, cleanup, retcode);
    }

    // Pop
    for (int i = n - 1; i > 0; i--)
    {
        int *ptr = array_pop(&array);
        TEST_ASSERT_GOTO(ptr != NULL, cleanup, retcode);
        int actual_x = *ptr;
        int expected_x = 2 * i * i - 1;
        TEST_ASSERT_GOTO(actual_x == expected_x, cleanup, retcode);
        TEST_ASSERT_GOTO(array.len == (unsigned)i, cleanup, retcode); // Silence sign mis-match
    }

cleanup:
    array_deinit(&array);
    return retcode;
}

int test_get_out_of_bounds(void)
{
    int retcode = TEST_SUCCESS;
    Array array = {0};
    int value;

    // Initialize
    value = array_init(&array, sizeof(int));
    TEST_ASSERT_GOTO(value == 0, cleanup, retcode);

    // Append
    size_t len = 32;
    for (size_t i = 0; i < len; i++)
    {
        value = array_append(&array, &i);
        TEST_ASSERT_GOTO(value == 0, cleanup, retcode);
    }

    // Get
    int *ptr = array_get(&array, len - 1);
    TEST_ASSERT_GOTO(ptr != NULL, cleanup, retcode);
    ptr = array_get(&array, len);
    TEST_ASSERT_GOTO(ptr == NULL, cleanup, retcode);

cleanup:
    array_deinit(&array);
    return retcode;
}

int test_shrink(void)
{
    int retcode = TEST_SUCCESS;
    Array array = {0};
    int value;

    // Initialize
    value = array_init(&array, sizeof(int));
    TEST_ASSERT_GOTO(value == 0, cleanup, retcode);

    // Shrink
    size_t capacity = array.capacity;
    array_shrink(&array);
    TEST_ASSERT_GOTO(array.capacity == capacity, cleanup, retcode);
    for (size_t i = 0; i < capacity + 1; i++)
        array_append(&array, &i);
    array_shrink(&array);
    TEST_ASSERT_GOTO(array.capacity == array.len, cleanup, retcode);

cleanup:
    array_deinit(&array);
    return retcode;
}

Test tests[] = {
    {&test_init, "test_init"},
    {&test_append_null_value, "append_null_value"},
    {&test_append_null_array, "append_null_array"},
    {&test_extend_null_values, "extend_null_values"},
    {&test_extend_null_array, "extend_null_array"},
    {&test_get_null, "get_null"},
    {&test_append_get, "test_append_get"},
    {&test_extend_get, "test_extend_get"},
    {&test_append_pop, "test_append_pop"},
    {&test_get_out_of_bounds, "test_get_out_of_bounds"},
    {&test_shrink, "test_shrink"},
};

#define NTESTS sizeof(tests) / sizeof(Test)

int main(void)
{
    run_tests(tests, NTESTS, MODULE_NAME);
}
