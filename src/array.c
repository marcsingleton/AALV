#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "array.h"

#define INIT_CAPACITY 16
#define EXPAND_FACTOR 2

Array *array_create(size_t size)
{
    Array *array = malloc(sizeof(Array));
    array->data = malloc(INIT_CAPACITY * size);
    array->size = size;
    array->capacity = INIT_CAPACITY;
    array->len = 0;

    return array;
}

int array_append(void *value, Array *array)
{
    if (array->len >= array->capacity)
    {
        void *ptr = realloc(array->data, EXPAND_FACTOR * array->capacity * array->size);
        if (ptr == NULL)
        {
            return -1;
        }
        array->data = ptr;
        array->capacity = EXPAND_FACTOR * array->capacity;
    }
    void *dst = (char *)array->data + array->len * array->size;
    memcpy(dst, value, array->size);
    (array->len)++;
    return 0;
}

void *array_get(size_t index, Array *array)
{
    if (index >= array->len)
    {
        return NULL;
    }
    return (char *)array->data + (index * array->size);
}