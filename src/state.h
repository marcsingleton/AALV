#ifndef STATE_H
#define STATE_H

/*
 * Program state
 */

#include <stdbool.h>

#include "array.h"
#include "color.h"
#include "pane.h"
#include "scroller.h"
#include "sequences.h"

#define RULER_PANE_ELLIPSES_CHAR "·"
#define RULER_PANE_ELLIPSES_NUM 3

#define RULER_PANE_MIN_HEIGHT RULER_PANE_ELLIPSES_NUM + 1
#define HEADER_PANE_MIN_WIDTH 2
#define SEQUENCE_PANE_MIN_WIDTH 2
#define RECORDS_PANE_MIN_HEIGHT 0
#define COMMAND_PANE_MIN_HEIGHT 2

#define SCROLLER_NPANES 2
#define SCROLLER_HEADER_PANE 0
#define SCROLLER_SEQUENCE_PANE 1

typedef struct
{
    unsigned int ruler_records_divider_i;
    unsigned int header_sequence_divider_j;
    unsigned int records_command_divider_i;
    unsigned int min_ruler_records_divider_i;
    unsigned int min_header_sequence_divider_j;
    unsigned int max_header_sequence_divider_j;
    unsigned int max_records_command_divider_i;
    Pane ruler_pane;
    Pane header_pane;
    Pane sequence_pane;
    Pane command_pane;
    RowLinkedScroller scroller;
} Layout;

typedef struct
{
    const char *file_path;
    Layout layout;
    unsigned int tick_offset;
    unsigned int tick_spacing;
    SeqRecordArray record_array;
    size_t records_max_len;
    UnalignedIndicesArray indices_array;
} FileState;

typedef struct
{
    const Alphabet *alphabet;
    ColorScheme *color_scheme;
} SeqTypeState;

typedef enum
{
    NORMAL = 0,
    COMMAND,
} Mode;

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
    bool visible_window;
    Mode mode;
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
    SeqTypeState *seq_types;
    unsigned int n_seq_types;
} State;

// FileState setters
void state_set_ruler_records_divider_i(State *state, unsigned int i);
void state_set_header_sequence_divider_j(State *state, unsigned int j);
void state_set_records_command_divider_i(State *state, unsigned int i);
void state_set_divider_limits(State *state);
void state_set_layout(State *state, unsigned int ruler_records_divider_i, unsigned int header_sequence_divider_j);
void state_set_tick_offset(State *state, unsigned int tick_offset);
void state_set_tick_spacing(State *state, unsigned int tick_spacing);

// State setters
void state_set_terminal_size(State *state);
void state_set_active_file_index(State *state, unsigned int file_index);
void state_set_seq_type_color_scheme(State *state, unsigned int seq_type_index, ColorScheme *color_scheme);

#endif // STATE_H
