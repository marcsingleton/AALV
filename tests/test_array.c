#include <assert.h>
#include <stdio.h>

#include "array.h"
#include "utils.h"

int test_init()
{
    Array array;
    int code = array_init(&array, sizeof(int));
    if (code != 0 || array.size != sizeof(int) || array.capacity == 0 || array.len != 0)
        return 1;
    else
        return 0;
}

int test_append_get()
{
    Array array;
    int code = array_init(&array, sizeof(int));
    if (code != 0)
        return 1;
    int n = 1024;
    for (int i = 0; i < n; i++)
    {
        code = array_append(&array, &i);
        if (code != 0)
            return 2;
    }
    for (int i = 0; i < 1024; i++)
    {
        int *ptr = array_get(&array, i);
        if (ptr == NULL)
            return 3;
        int x = *ptr;
        if (x != i)
            return 4;
    }
    return 0;
}

int test_get_out_of_bounds()
{
    Array array;
    int code = array_init(&array, sizeof(int));
    size_t len = 32;
    if (code != 0)
        return 1;
    for (int i = 0; i < len; i++)
    {
        code = array_append(&array, &i);
        if (code != 0)
            return 2;
    }
    int *ptr = array_get(&array, len - 1);
    if (ptr == NULL)
        return 3;
    ptr = array_get(&array, len);
    if (ptr != NULL)
        return 4;
    return 0;
}

TestFunction tests[] = {
    {&test_init, "test_init"},
    {&test_append_get, "test_append"},
    {&test_get_out_of_bounds, "test_get_out_of_bounds"},
};

#define NTESTS sizeof(tests) / sizeof(TestFunction)

int main()
{
    run_tests(tests, NTESTS, "test_array.c");
}