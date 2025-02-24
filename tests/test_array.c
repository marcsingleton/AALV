#include <assert.h>
#include <stdio.h>

#include "array.h"
#include "utils.h"

int test_init()
{
    Array array;
    array_init(&array, sizeof(int));
    if (array.size != sizeof(int) || array.capacity == 0 || array.len != 0)
        return -1;
    else
        return 0;
}

int test_append_get()
{
    Array array;
    array_init(&array, sizeof(int));
    int n = 1024;
    for (int i = 0; i < n; i++)
    {
        array_append(&array, &i);
    }
    for (int i = 0; i < 1024; i++)
    {
        int x = *(int *)array_get(&array, i);
    }
    return 0;
}

TestFunction tests[] = {
    {&test_init, "test_init"},
    {&test_append_get, "test_append"}};

#define NTESTS sizeof(tests) / sizeof(TestFunction)

int main()
{
    for (int i = 0; i < NTESTS; i++)
    {
        TestFunction test = tests[i];
        printf("Testing function %s\n", test.func_name);
        int code = (*test.func_ptr)();
        if (code == 0)
            printf("    Passed!\n");
        else
            printf("    Failed with code %d\n", code);
    }
}