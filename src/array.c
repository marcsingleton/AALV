#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "array.h"

#define INIT_CAPACITY 16
#define EXPAND_FACTOR 2

Array *create_array(size_t size)
{
    Array *array = malloc(sizeof(Array));
    array->data = malloc(INIT_CAPACITY * size);
    array->size = size;
    array->capacity = INIT_CAPACITY;
    array->len = 0;

    return array;
}

void append_array(void *value, Array *array)
{
    if (array->len < array->capacity)
    {
        void *dst = (char *)array->data + (array->len) * (array->size);
        memcpy(dst, value, array->size);
        (array->len)++;
    }
    else
    {
        printf("Array is full!\n");
    }
}

void *get_array(size_t index, Array *array)
{
    if (index >= array->len)
    {
        return NULL;
    }
    return (char *)array->data + (index * array->size);
}