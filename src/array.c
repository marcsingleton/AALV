#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "array.h"

#define INIT_CAPACITY 16
#define EXPAND_FACTOR 2

int array_init(Array *array, size_t size)
{
    void *ptr = malloc(INIT_CAPACITY * size);
    if (ptr == NULL)
        return 1;
    array->data = malloc(INIT_CAPACITY * size);
    array->size = size;
    array->capacity = INIT_CAPACITY;
    array->len = 0;
    return 0;
}

void array_free(Array *array)
{
    free(array->data);
    array->data = NULL;
    array->size = 0;
    array->capacity = 0;
    array->len = 0;
}

int array_append(Array *array, void *value)
{
    if (array->len >= array->capacity)
    {
        void *ptr = realloc(array->data, EXPAND_FACTOR * array->capacity * array->size);
        if (ptr == NULL)
        {
            return 1;
        }
        array->data = ptr;
        array->capacity = EXPAND_FACTOR * array->capacity;
    }
    void *dst = (char *)array->data + array->len * array->size;
    memcpy(dst, value, array->size);
    (array->len)++;
    return 0;
}

void *array_get(Array *array, size_t index)
{
    if (index >= array->len)
    {
        return NULL;
    }
    return (char *)array->data + (index * array->size);
}