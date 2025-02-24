#ifndef ARRAY_H
#define ARRAY_H

/*
 * Dynamic arrays
 */

typedef struct
{
    void *data;
    size_t size;
    size_t capacity;
    size_t len;
} Array;

void array_init(Array *array, size_t size);
void array_free(Array *array);
int array_append(Array *array, void *value);
void *array_get(Array *array, size_t index);

#endif // ARRAY_H
