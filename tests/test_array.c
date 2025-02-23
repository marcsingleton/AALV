#include <assert.h>
#include <stdio.h>

#include "array.h"

void test_init_capacity()
{
    Array *array = create_array(1);
}

void test_append()
{
    Array *array = create_array(sizeof(1));
    assert(array->capacity == 16);
    printf("Assertion successful!\n");
    for (int i = 0; i < 17; i++)
    {
        append_array(&i, array);
    }
    for (int i = 0; i < 17; i++)
    {
        int x = *(int *)get_array(i, array);
        printf("Got element %d\n", x);
    }
}

int main()
{
    test_append();
}