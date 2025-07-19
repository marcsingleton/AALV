#ifndef STATE_H
#define STATE_H

/*
 * Program state
 */

#include <stdbool.h>

#include "array.h"
#include "color.h"
#include "sequences.h"

typedef struct
{
    unsigned int header_pane_width;
    unsigned int ruler_pane_height;
    unsigned int tick_spacing;
    unsigned int offset_record;
    unsigned int offset_header;
    unsigned int offset_sequence;
    unsigned int cursor_record_i;   // Row index in header/sequence panes
    unsigned int cursor_header_j;   // Column index in header pane
    unsigned int cursor_sequence_j; // Column index in sequence pane
    SeqRecordArray record_array;
} FileState;

typedef struct
{
    const Alphabet *alphabet;
    ColorScheme *color_scheme;
} SeqTypeState;

typedef struct
{
    // Global state variables
    unsigned int terminal_rows;
    unsigned int terminal_cols;
    bool refresh_ruler_pane;
    bool refresh_header_pane;
    bool refresh_sequence_pane;
    bool refresh_command_pane;
    bool refresh_window;
    // File state variables
    FileState *files;
    unsigned int nfiles;
    FileState *active_file;
    unsigned int active_file_index;
    // Color variables
    ColorScheme *color_schemes;
    unsigned int n_color_schemes;
    int ncolors;
    // Type variables
    SeqTypeState *types;
    unsigned int ntypes;
} State;

// FileState setters
void state_set_header_pane_width(State *state, unsigned int header_pane_width);
void state_set_ruler_pane_height(State *state, unsigned int ruler_pane_height);
void state_set_tick_spacing(State *state, unsigned int tick_spacing);
void state_set_offset_record(State *state, unsigned int offset_record);
void state_set_offset_header(State *state, unsigned int offset_header);
void state_set_offset_sequence(State *state, unsigned int offset_sequence);
inline void state_set_cursor_record_i(State *state, unsigned int cursor_record_i);
inline void state_set_cursor_header_j(State *state, unsigned int cursor_header_j);
inline void state_set_cursor_sequence_j(State *state, unsigned int cursor_sequence_j);

// FileState getters
unsigned int state_get_record_panes_height(State *state);
unsigned int state_get_sequence_pane_width(State *state);

// State setters
void state_set_active_file_index(State *state, unsigned int active_file_index);

#endif // STATE_H
