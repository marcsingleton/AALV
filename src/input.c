#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/select.h>
#include <unistd.h>

#include "array.h"
#include "input.h"
#include "state.h"
#include "terminal.h"

extern State state;

int input_read_key(Array *buffer, int fd)
{
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(fd, &readfds);

    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 2500;

    char c;
    int n = 0;
    while (select(fd + 1, &readfds, NULL, NULL, &tv) > 0 && FD_ISSET(fd, &readfds))
    {
        read(fd, &c, 1);
        array_append(buffer, &c);
        n++;
    }

    return n;
}

int input_parse_keys(Array *buffer, int *count, Command *cmd)
{
    /* Return codes
        0: success
        1: incomplete
        2: failure
    */

    size_t index = 0;
    char *ptr, c;

    // Parse digits
    size_t a = 0;
    int default_count = 0;
    while (index < buffer->len)
    {
        ptr = array_get(buffer, index);
        c = *ptr;
        if (!isdigit(c))
            break;
        if (a == 0 && c == '0') // 0 is line start, so ignore at start of digits
            break;
        if (a > (SIZE_MAX - (c - '0')) / 10)
            return 2;
        a = 10 * a + (c - '0');
        index++;
    }
    if (index >= buffer->len)
        return 1;
    if (a == 0)
    {
        a = 1;
        default_count = 1;
    }

    // Parse command
    switch (c)
    {
    case 27: // ESC
        if (index + 2 >= buffer->len)
            return 2;
        ptr = array_get(buffer, index + 1);
        c = *ptr;
        if (c != '[')
            return 2;
        ptr = array_get(buffer, index + 2);
        c = *ptr;
        switch (c)
        {
        case 'A':
            *cmd = CMD_MOVE_UP;
            break;
        case 'B':
            *cmd = CMD_MOVE_DOWN;
            break;
        case 'C':
            *cmd = CMD_MOVE_RIGHT;
            break;
        case 'D':
            *cmd = CMD_MOVE_LEFT;
            break;
        default:
            return 2;
        }
        break;
    case 'q':
        *cmd = CMD_QUIT;
        break;
    case '>':
        *cmd = CMD_NEXT_FILE;
        break;
    case '<':
        *cmd = CMD_PREVIOUS_FILE;
        break;
    case 'k':
        *cmd = CMD_MOVE_UP;
        break;
    case 'j':
        *cmd = CMD_MOVE_DOWN;
        break;
    case 'l':
        *cmd = CMD_MOVE_RIGHT;
        break;
    case 'h':
        *cmd = CMD_MOVE_LEFT;
        break;
    case CTRL('b'):
        *cmd = CMD_MOVE_FULL_PAGE_UP;
        break;
    case CTRL('f'):
        *cmd = CMD_MOVE_FULL_PAGE_DOWN;
        break;
    case CTRL('n'):
        *cmd = CMD_MOVE_FULL_PAGE_RIGHT;
        break;
    case CTRL('p'):
        *cmd = CMD_MOVE_FULL_PAGE_LEFT;
        break;
    case CTRL('u'):
        *cmd = CMD_MOVE_HALF_PAGE_UP;
        break;
    case CTRL('d'):
        *cmd = CMD_MOVE_HALF_PAGE_DOWN;
        break;
    case CTRL('r'):
        *cmd = CMD_MOVE_HALF_PAGE_RIGHT;
        break;
    case CTRL('l'):
        *cmd = CMD_MOVE_HALF_PAGE_LEFT;
        break;
    case '$':
        *cmd = CMD_MOVE_LINE_END;
        break;
    case '0':
    case '^':
        *cmd = CMD_MOVE_LINE_START;
        break;
    case 'g':
        if (index + 1 >= buffer->len)
            return 1;
        ptr = array_get(buffer, index + 1);
        c = *ptr;
        if (c != 'g')
            return 2;
        *cmd = CMD_MOVE_FIRST_RECORD;
        break;
    case 'G':
        if (default_count == 1)
            *cmd = CMD_MOVE_LAST_RECORD;
        else
        {
            *cmd = CMD_MOVE_TO_RECORD;
            a--;
        }
        break;
    case 'H':
        *cmd = CMD_MOVE_TOP_EDGE;
        break;
    case 'M':
        *cmd = CMD_MOVE_VERTICAL_MIDDLE;
        break;
    case 'L':
        *cmd = CMD_MOVE_BOTTOM_EDGE;
        break;
    case 'S':
        *cmd = CMD_MOVE_LEFT_EDGE;
        break;
    case 'C':
        *cmd = CMD_MOVE_HORIZONTAL_MIDDLE;
        break;
    case 'E':
        *cmd = CMD_MOVE_RIGHT_EDGE;
        break;
    case ']':
        *cmd = CMD_INCREASE_HEADER_PANE_WIDTH;
        break;
    case '[':
        *cmd = CMD_DECREASE_HEADER_PANE_WIDTH;
        break;
    case '}':
        *cmd = CMD_INCREASE_RULER_PANE_HEIGHT;
        break;
    case '{':
        *cmd = CMD_DECREASE_RULER_PANE_HEIGHT;
        break;
    case '+':
        *cmd = CMD_INCREASE_TICK_SPACING;
        break;
    case '-':
        *cmd = CMD_DECREASE_TICK_SPACING;
        break;
    default:
        return 2;
    }

    *count = a;

    return 0;
}

int input_execute_command(int count, Command cmd)
{
    switch (cmd)
    {
    case CMD_QUIT:
        exit(0);
        break;
    case CMD_NEXT_FILE:
        input_next_file();
        break;
    case CMD_PREVIOUS_FILE:
        input_previous_file();
        break;
    case CMD_MOVE_DOWN:
        input_move_down(count);
        break;
    case CMD_MOVE_UP:
        input_move_up(count);
        break;
    case CMD_MOVE_RIGHT:
        input_move_right(count);
        break;
    case CMD_MOVE_LEFT:
        input_move_left(count);
        break;
    case CMD_MOVE_FULL_PAGE_UP:
        input_move_page_up(PAGE_SIZE_FULL);
        break;
    case CMD_MOVE_FULL_PAGE_DOWN:
        input_move_page_down(PAGE_SIZE_FULL);
        break;
    case CMD_MOVE_FULL_PAGE_RIGHT:
        input_move_page_right(PAGE_SIZE_FULL);
        break;
    case CMD_MOVE_FULL_PAGE_LEFT:
        input_move_page_left(PAGE_SIZE_FULL);
        break;
    case CMD_MOVE_HALF_PAGE_UP:
        input_move_page_up(PAGE_SIZE_HALF);
        break;
    case CMD_MOVE_HALF_PAGE_DOWN:
        input_move_page_down(PAGE_SIZE_HALF);
        break;
    case CMD_MOVE_HALF_PAGE_RIGHT:
        input_move_page_right(PAGE_SIZE_HALF);
        break;
    case CMD_MOVE_HALF_PAGE_LEFT:
        input_move_page_left(PAGE_SIZE_HALF);
        break;
    case CMD_MOVE_LINE_END:
        input_move_line_end();
        break;
    case CMD_MOVE_LINE_START:
        input_move_line_start();
        break;
    case CMD_MOVE_FIRST_RECORD:
        input_move_first_record();
        break;
    case CMD_MOVE_LAST_RECORD:
        input_move_last_record();
        break;
    case CMD_MOVE_TO_RECORD:
        input_move_to_record(count);
        break;
    case CMD_MOVE_TOP_EDGE:
        input_move_top_edge();
        break;
    case CMD_MOVE_VERTICAL_MIDDLE:
        input_move_vertical_middle();
        break;
    case CMD_MOVE_BOTTOM_EDGE:
        input_move_bottom_edge();
        break;
    case CMD_MOVE_LEFT_EDGE:
        input_move_left_edge();
        break;
    case CMD_MOVE_HORIZONTAL_MIDDLE:
        input_move_horizontal_middle();
        break;
    case CMD_MOVE_RIGHT_EDGE:
        input_move_right_edge();
        break;
    case CMD_INCREASE_HEADER_PANE_WIDTH:
        input_increase_header_pane_width();
        break;
    case CMD_DECREASE_HEADER_PANE_WIDTH:
        input_decrease_header_pane_width();
        break;
    case CMD_INCREASE_RULER_PANE_HEIGHT:
        input_increase_ruler_pane_height();
        break;
    case CMD_DECREASE_RULER_PANE_HEIGHT:
        input_decrease_ruler_pane_height();
        break;
    case CMD_INCREASE_TICK_SPACING:
        input_increase_tick_spacing();
        break;
    case CMD_DECREASE_TICK_SPACING:
        input_decrease_tick_spacing();
        break;
    }

    return 0;
}

void input_buffer_flush(Array *buffer)
{
    write(STDOUT_FILENO, buffer->data, buffer->len);
    buffer->len = 0;
}

void input_next_file(void)
{
    if (state.active_file_index + 1 >= state.nfiles)
        return;
    state_set_active_file_index(&state, state.active_file_index + 1);
    state.refresh_window = true;
}

void input_previous_file(void)
{
    if (state.active_file_index == 0)
        return;
    state_set_active_file_index(&state, state.active_file_index - 1);
    state.refresh_window = true;
}

void input_cursor_clamp(void)
{
    FileState *active_file = state.active_file;
    unsigned int record_panes_height = state_get_record_panes_height(&state);
    unsigned int sequence_pane_width = state_get_sequence_pane_width(&state);
    if (record_panes_height == 0)
        record_panes_height = 1; // Treat collapsed pane as single row

    if (active_file->cursor_record_i + 1 > record_panes_height)
        active_file->cursor_record_i = record_panes_height - 1;
    if (active_file->cursor_sequence_j > sequence_pane_width)
        active_file->cursor_sequence_j = sequence_pane_width - 1;
}

void input_move_up(size_t x)
{
    input_cursor_clamp();
    FileState *active_file = state.active_file;

    if (active_file->nrecords == 0)
        return;

    size_t record_index = active_file->cursor_record_i + active_file->offset_record;
    if (x > record_index)
        x = record_index;
    if (x > active_file->cursor_record_i)
    {
        active_file->cursor_record_i = 0;
        state_set_offset_record(&state, record_index - x);
    }
    else
        active_file->cursor_record_i -= x;
}

void input_move_down(size_t x)
{
    input_cursor_clamp();
    FileState *active_file = state.active_file;

    if (active_file->nrecords == 0)
        return;

    unsigned int record_panes_height = state_get_record_panes_height(&state);
    if (record_panes_height == 0)
        record_panes_height = 1; // Treat collapsed pane as single row

    size_t record_index = active_file->cursor_record_i + active_file->offset_record;
    if (x + record_index + 1 >= active_file->nrecords)
        x = active_file->nrecords - record_index - 1;
    if (x + active_file->cursor_record_i + 1 > record_panes_height)
    {
        x += active_file->cursor_record_i - record_panes_height + 1;
        active_file->cursor_record_i = record_panes_height - 1;
        state_set_offset_record(&state, active_file->offset_record + x);
    }
    else
        active_file->cursor_record_i += x;
}

void input_move_right(size_t x)
{
    input_cursor_clamp();
    FileState *active_file = state.active_file;

    if (active_file->nrecords == 0)
        return;

    unsigned int sequence_pane_width = state_get_sequence_pane_width(&state);

    size_t record_index = active_file->cursor_record_i + active_file->offset_record;
    SeqRecord record = active_file->records[record_index];
    size_t sequence_index = active_file->cursor_sequence_j + active_file->offset_sequence;

    // Snap to end
    if (sequence_index + 1 > record.len)
    {
        sequence_index = (record.len > 0) ? record.len - 1 : 0;
        if (sequence_index < active_file->offset_sequence)
        {
            active_file->cursor_sequence_j = 0;
            state_set_offset_sequence(&state, sequence_index);
        }
        else
            active_file->cursor_sequence_j = sequence_index - active_file->offset_sequence;
    }

    // Move
    if (record.len == 0)
        return;
    if (x + sequence_index + 1 >= record.len)
        x = record.len - sequence_index - 1;
    if (x + active_file->cursor_sequence_j + 1 > sequence_pane_width)
    {
        x += active_file->cursor_sequence_j - sequence_pane_width + 1;
        active_file->cursor_sequence_j = sequence_pane_width - 1;
        state_set_offset_sequence(&state, active_file->offset_sequence + x);
    }
    else
        active_file->cursor_sequence_j += x;
}

void input_move_left(size_t x)
{
    input_cursor_clamp();
    FileState *active_file = state.active_file;

    if (active_file->nrecords == 0)
        return;

    size_t record_index = active_file->cursor_record_i + active_file->offset_record;
    SeqRecord record = active_file->records[record_index];
    size_t sequence_index = active_file->cursor_sequence_j + active_file->offset_sequence;

    // Snap to end
    if (sequence_index + 1 > record.len)
    {
        sequence_index = (record.len > 0) ? record.len - 1 : 0;
        if (sequence_index < active_file->offset_sequence)
        {
            active_file->cursor_sequence_j = 0;
            state_set_offset_sequence(&state, sequence_index);
        }
        else
            active_file->cursor_sequence_j = sequence_index - active_file->offset_sequence;
    }

    // Move
    if (x > sequence_index)
        x = sequence_index;
    if (x > active_file->cursor_sequence_j)
    {
        active_file->cursor_sequence_j = 0;
        state_set_offset_sequence(&state, sequence_index - x);
    }
    else
        active_file->cursor_sequence_j -= x;
}

void input_move_page_up(PageSize page_size)
{
    input_cursor_clamp();
    FileState *active_file = state.active_file;

    if (active_file->nrecords == 0)
        return;

    unsigned int record_panes_height = state_get_record_panes_height(&state);
    if (record_panes_height == 0)
        record_panes_height = 1; // Treat collapsed pane as single row

    size_t x = record_panes_height;
    if (page_size == PAGE_SIZE_HALF)
        x /= 2;
    if (active_file->offset_record == 0)
        return;
    else if (x > active_file->offset_record)
        state_set_offset_record(&state, 0);
    else
        state_set_offset_record(&state, active_file->offset_record - x);
}

void input_move_page_down(PageSize page_size)
{
    input_cursor_clamp();
    FileState *active_file = state.active_file;

    if (active_file->nrecords == 0)
        return;

    unsigned int record_panes_height = state_get_record_panes_height(&state);
    if (record_panes_height == 0)
        record_panes_height = 1; // Treat collapsed pane as single row

    unsigned int x = record_panes_height;
    if (page_size == PAGE_SIZE_HALF)
        x /= 2;
    if (active_file->offset_record + 1 == active_file->nrecords)
        return;
    else if (active_file->offset_record + x + 1 > active_file->nrecords)
        state_set_offset_record(&state, active_file->nrecords - 1);
    else
        state_set_offset_record(&state, active_file->offset_record + x);
    if (active_file->offset_record + active_file->cursor_record_i + 1 > active_file->nrecords)
        active_file->cursor_record_i = active_file->nrecords - 1 - active_file->offset_record;
}

void input_move_page_right(PageSize page_size)
{
    input_cursor_clamp();
    FileState *active_file = state.active_file;

    if (active_file->nrecords == 0)
        return;

    unsigned int x = state_get_sequence_pane_width(&state);
    if (page_size == PAGE_SIZE_HALF)
        x /= 2;
    if (active_file->offset_sequence + x + 2 > active_file->records_maxlen) // Accounts for continuation symbol
        state_set_offset_sequence(&state, active_file->records_maxlen - 2);
    else
        state_set_offset_sequence(&state, active_file->offset_sequence + x);
}

void input_move_page_left(PageSize page_size)
{
    input_cursor_clamp();
    FileState *active_file = state.active_file;

    if (active_file->nrecords == 0)
        return;

    unsigned int x = state_get_sequence_pane_width(&state);
    if (page_size == PAGE_SIZE_HALF)
        x /= 2;
    if (x > active_file->offset_sequence)
        state_set_offset_sequence(&state, 0);
    else
        state_set_offset_sequence(&state, active_file->offset_sequence - x);
}

void input_move_line_start(void)
{
    input_cursor_clamp();
    FileState *active_file = state.active_file;

    if (active_file->nrecords == 0)
        return;

    state_set_offset_sequence(&state, 0);
    active_file->cursor_sequence_j = 0;
}

void input_move_line_end(void)
{
    input_cursor_clamp();
    FileState *active_file = state.active_file;

    if (active_file->nrecords == 0)
        return;

    size_t record_index = active_file->cursor_record_i + active_file->offset_record;
    SeqRecord record = active_file->records[record_index];
    size_t sequence_index = active_file->cursor_sequence_j + active_file->offset_sequence;
    size_t x = (record.len > 0) ? record.len - 1 - sequence_index : 0;
    input_move_right(x);
}

void input_move_first_record(void)
{
    input_cursor_clamp();
    FileState *active_file = state.active_file;

    if (active_file->nrecords == 0)
        return;

    active_file->cursor_record_i = 0;
    state_set_offset_record(&state, 0);
}

void input_move_last_record(void)
{
    input_cursor_clamp();
    FileState *active_file = state.active_file;

    if (active_file->nrecords == 0)
        return;

    size_t record_index = active_file->cursor_record_i + active_file->offset_record;
    size_t x = active_file->nrecords - 1 - record_index;
    input_move_down(x);
}

void input_move_to_record(size_t x)
{
    input_cursor_clamp();
    FileState *active_file = state.active_file;

    if (active_file->nrecords == 0)
        return;

    size_t record_index = active_file->cursor_record_i + active_file->offset_record;
    if (x > record_index)
        input_move_down(x - record_index);
    else if (x < record_index)
        input_move_up(record_index - x);
}

void input_move_bottom_edge(void)
{
    input_cursor_clamp();
    FileState *active_file = state.active_file;

    if (active_file->nrecords == 0)
        return;

    unsigned int record_panes_height = state_get_record_panes_height(&state);
    if (active_file->offset_record + record_panes_height > active_file->nrecords)
        active_file->cursor_record_i = active_file->nrecords - active_file->offset_record - 1;
    else
        active_file->cursor_record_i = record_panes_height - 1;
}

void input_move_top_edge(void)
{
    input_cursor_clamp();
    FileState *active_file = state.active_file;

    if (active_file->nrecords == 0)
        return;

    active_file->cursor_record_i = 0;
}

void input_move_left_edge(void)
{
    input_cursor_clamp();
    FileState *active_file = state.active_file;

    if (active_file->nrecords == 0)
        return;

    active_file->cursor_sequence_j = 0;
}

void input_move_right_edge(void)
{
    input_cursor_clamp();
    FileState *active_file = state.active_file;

    if (active_file->nrecords == 0)
        return;

    active_file->cursor_sequence_j = state_get_sequence_pane_width(&state) - 1;
}

void input_move_vertical_middle(void)
{
    input_cursor_clamp();
    FileState *active_file = state.active_file;

    if (active_file->nrecords == 0)
        return;

    unsigned int record_panes_height = state_get_record_panes_height(&state);
    if (active_file->offset_record + record_panes_height > active_file->nrecords)
        active_file->cursor_record_i = (active_file->nrecords - active_file->offset_record - 1) / 2;
    else
        active_file->cursor_record_i = (record_panes_height - 1) / 2;
}

void input_move_horizontal_middle(void)
{
    input_cursor_clamp();
    FileState *active_file = state.active_file;

    if (active_file->nrecords == 0)
        return;

    unsigned int sequence_pane_width = state_get_sequence_pane_width(&state);
    active_file->cursor_sequence_j = sequence_pane_width / 2;
}

void input_increase_header_pane_width(void)
{
    FileState *active_file = state.active_file;
    state_set_header_pane_width(&state, active_file->header_pane_width + 1);
}

void input_decrease_header_pane_width(void)
{
    FileState *active_file = state.active_file;
    state_set_header_pane_width(&state, active_file->header_pane_width - 1);
}

void input_increase_ruler_pane_height(void)
{
    FileState *active_file = state.active_file;
    state_set_ruler_pane_height(&state, active_file->ruler_pane_height + 1);
}

void input_decrease_ruler_pane_height(void)
{
    FileState *active_file = state.active_file;
    state_set_ruler_pane_height(&state, active_file->ruler_pane_height - 1);
}

void input_increase_tick_spacing(void)
{
    FileState *active_file = state.active_file;
    state_set_tick_spacing(&state, active_file->tick_spacing + 1);
}

void input_decrease_tick_spacing(void)
{
    FileState *active_file = state.active_file;
    state_set_tick_spacing(&state, active_file->tick_spacing - 1);
}
