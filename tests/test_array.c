#include <stdio.h>

#include "array.h"

void test_append()
{
    Array *array = create_array(1);
    printf("Ran test array");
}

int main()
{
    test_append();
}