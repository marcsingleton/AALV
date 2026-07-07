#ifndef IO_H
#define IO_H

/*
 * IO
 */

#include "sequences.h"
#include "state.h"

int io_load_seqs(FileState *file, const char *format_arg);
int io_set_seq_types(FileState *file, const char *seq_type_arg);
int io_set_unaligned_indices(FileState *file);

#endif // IO_H
