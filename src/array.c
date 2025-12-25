#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "array.h"

#define INIT_CAPACITY 16
#define EXPAND_FACTOR 2
#define CONTRACT_FACTOR 4

int array_init(Array *array, size_t size)
{
    if (!array || size == 0)
        return 1;
    void *data = malloc(INIT_CAPACITY * size);
    if (!data)
        return 1;
    array->data = data;
    array->size = size;
    array->capacity = INIT_CAPACITY;
    array->max_capacity = SIZE_MAX / size;
    array->len = 0;
    return 0;
}

void array_deinit(Array *array)
{
    if (!array)
        return;
    free(array->data);
    array->data = NULL;
    array->size = 0;
    array->capacity = 0;
    array->max_capacity = 0;
    array->len = 0;
}

Array *array_create(size_t size)
{
    Array *array = malloc(sizeof(Array));
    if (!array)
        return NULL;
    int retcode = array_init(array, size);
    if (retcode > 0)
    {
        free(array);
        return NULL;
    }
    return array;
}

void array_destroy(Array *array)
{
    array_deinit(array);
    free(array);
}

int array_append(Array *array, const void *value)
{
    if (!array)
        return 1;
    if (array->len >= array->capacity)
    {
        if (array->capacity > array->max_capacity / EXPAND_FACTOR)
            return 1;
        size_t new_capacity = EXPAND_FACTOR * array->capacity;
        void *ptr = realloc(array->data, new_capacity * array->size);
        if (!ptr)
            return 1;
        array->data = ptr;
        array->capacity = new_capacity;
    }
    void *dst = (char *)array->data + array->len * array->size;
    memcpy(dst, value, array->size);
    (array->len)++;
    return 0;
}

int array_extend(Array *array, const void *values, size_t len)
{
    if (!array || !values)
        return 1;
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
        if (!ptr)
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
    if (!array)
        return NULL;
    if (index >= array->len)
        return NULL;
    return (char *)array->data + (index * array->size);
}

void *array_pop(Array *array)
{
    if (!array)
        return NULL;
    if (array->len == 0)
        return NULL;
    size_t index = array->len - 1;
    array->len--;
    return (char *)array->data + (index * array->size);
}

int array_shrink(Array *array)
{
    if (!array)
        return 1;
    size_t capacity = (array->len == 0) ? INIT_CAPACITY : array->len;
    void *ptr = realloc(array->data, capacity * array->size);
    if (!ptr)
        return 1;
    array->data = ptr;
    array->capacity = capacity;
    return 0;
}
