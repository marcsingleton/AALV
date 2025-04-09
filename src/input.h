#ifndef INPUT_H
#define INPUT_H

/*
 * Input functions
 */

#include "array.h"

int input_get_action(void);
int input_process_action(int action, Array *buffer);
void input_buffer_flush(Array *buffer);

#endif // INPUT_H
