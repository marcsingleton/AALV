#ifndef STATE_H
#define STATE_H

/*
 * Program state
 */

#include <stdbool.h>

#include "color.h"
#include "config.h"
#include "pane.h"
#include "schemes.h"
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
    unsigned int ruler_records_divider;
    unsigned int header_sequence_divider;
    unsigned int records_command_divider;
    unsigned int min_ruler_records_divider;
    unsigned int min_header_sequence_divider;
    unsigned int max_header_sequence_divider;
    unsigned int max_records_command_divider;
    Pane ruler_pane;
    Pane header_pane;
    Pane sequence_pane;
    Pane command_pane;
    RowLinkedScroller scroller;
} Layout;

typedef struct
{
    size_t max_len;
    UnalignedIndicesArray indices_array;
} FileMetadata;

typedef struct
{
    char *file_path;
    Layout layout;
    int tick_offset;
    int tick_spacing;
    SeqRecordArray record_array;
    FileMetadata metadata;
} FileState;

typedef enum
{
    NORMAL = 0,
    COMMAND,
} Mode;

typedef struct
{
    // Global state
    Config config;
    Mode mode;
    unsigned int terminal_rows;
    unsigned int terminal_cols;
    bool refresh_ruler_pane;
    bool refresh_header_pane;
    bool refresh_sequence_pane;
    bool refresh_command_pane;
    bool refresh_window;
    bool visible_window;
    // File state
    FileState *files;
    unsigned int nfiles;
    FileState *active_file;
    unsigned int active_file_index;
    // Color
    int ncolors;
    SeqColorScheme *color_schemes;
    unsigned int n_color_schemes;
    SeqColorScheme **active_color_schemes;
    unsigned int n_active_color_schemes;
} State;

extern SeqColorScheme *active_color_schemes[];
extern unsigned int n_active_color_schemes;

// FileState
int state_init_file_data(FileState *file);
void state_deinit_file_data(FileState *file);
int state_set_file_path(FileState *file, char *file_path);

// FileState and State
int state_set_ruler_records_divider(State *state, unsigned int i);
int state_set_header_sequence_divider(State *state, unsigned int j);
int state_set_records_command_divider(State *state, unsigned int i);
int state_set_divider_limits(State *state);
int state_set_layout(State *state, unsigned int ruler_records_divider, unsigned int header_sequence_divider);
int state_set_tick_offset(State *state, int tick_offset);
int state_set_tick_spacing(State *state, int tick_spacing);

// State
int state_init(State *state);
void state_deinit(State *state);
int state_set_terminal_size(State *state);
int state_set_active_file_index(State *state, unsigned int file_index);
int state_new_file(State *state);
int state_remove_file(State *state);
int state_set_active_color_scheme(State *state, SeqColorScheme *color_scheme);
int state_new_color_scheme(State *state, char *name, SeqType seq_type, ColorType color_type);
int state_remove_color_scheme(State *state, char *name);

#endif // STATE_H
