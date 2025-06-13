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
    array->max_capacity = SIZE_MAX / size;
    array->len = 0;
    return 0;
}

void array_free(Array *array)
{
    free(array->data);
    array->data = NULL;
    array->size = 0;
    array->capacity = 0;
    array->max_capacity = 0;
    array->len = 0;
}

int array_append(Array *array, void *value)
{
    if (array->len >= array->capacity)
    {
        if (array->capacity > array->max_capacity / EXPAND_FACTOR)
            return 1;
        size_t new_capacity = EXPAND_FACTOR * array->capacity;
        void *ptr = realloc(array->data, new_capacity * array->size);
        if (ptr == NULL)
            return 1;
        array->data = ptr;
        array->capacity = new_capacity;
    }
    void *dst = (char *)array->data + array->len * array->size;
    memcpy(dst, value, array->size);
    (array->len)++;
    return 0;
}

int array_extend(Array *array, void *values, size_t len)
{
    if (array->len > array->max_capacity - len)
        return 1;
    size_t new_len = array->len + len;
    if (new_len > array->capacity)
    {
        size_t new_capacity = array->capacity;
        while (new_len > new_capacity)
        {
            if (new_capacity > array->max_capacity / EXPAND_FACTOR)
                return 1;
            new_capacity *= EXPAND_FACTOR;
        }
        void *ptr = realloc(array->data, new_capacity * array->size);
        if (ptr == NULL)
            return 1;
        array->data = ptr;
        array->capacity = new_capacity;
    }
    void *dst = (char *)array->data + array->len * array->size;
    memcpy(dst, values, len * array->size);
    array->len = new_len;
    return 0;
}

void *array_get(Array *array, size_t index)
{
    if (index >= array->len)
        return NULL;
    return (char *)array->data + (index * array->size);
}

int array_shrink(Array *array)
{
    size_t capacity = (array->len == 0) ? INIT_CAPACITY : array->len;
    void *ptr = realloc(array->data, capacity * array->size);
    if (ptr == NULL)
        return 1;
    array->data = ptr;
    array->capacity = capacity;
    return 0;
}
