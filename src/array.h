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

Array *create_array(size_t size);
void append_array(void *value, Array *array);
void *get_array(size_t index, Array *array);

#endif // ARRAY_H
