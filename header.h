#ifndef SC64_H
#define SC64_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NAME "Screwaround64"

#define MAIN_WND 0
#define ASM_WND  1
#define BIN_WND  2
#define ASM_HEAD 3
#define BIN_HEAD 4

#define N_WNDS   5

#define DISP_FONT 0

#define WIDTH  640
#define HEIGHT 480

#define HEADING_Y 27

#define X_OFF   20
#define Y_OFF   50
#define MID_GAP 20
#define BOTTOM 100

#define MAX_BIN_WIDTH 300

#define BORDER 2
#define MARGIN 3
#define CARET  2

#define BLACK      RGB(0x00, 0x00, 0x00)
#define WHITE      RGB(0xff, 0xff, 0xff)
#define BACKGROUND RGB(0xe0, 0xe0, 0xe0)

#define LINE_MAX 64

typedef unsigned char u8;
typedef unsigned int u32;

struct line_t {
	struct line_t *next;
	struct line_t *prev;
	struct line_t *equiv;
	char *str;
	int start, end;
	int col;
};

typedef struct line_t line_t;
typedef void(*line_func)(line_t*);

typedef struct {
	int type; // 1 == ASM_WND, 2 == BIN_WND

	line_t *first, *last; // First and last lines in the list
	line_t *top;          // First and last VISIBLE lines
	line_t *cur;          // Current line
	line_t *start, *end;  // Currently selected lines, from 'start' to 'end'

	int column;
	int sel; // Selection type: 0 = no selection, 1 = single line, 2 = start before end, 3 = end before start

	// markings/comment information go here...
} text_t;

typedef struct {
	char *title;
	char *desc;
	u32 ram;
	u32 rom;
	text_t asm_text; // not pointers
	text_t bin_text;
} tab_t;

typedef struct {
	char *name;
	char *title;
	char *desc;

	tab_t *tab;
	int n_tabs;
	int idx;

	// properties go here...
} project_t;

// asm.c

void assemble(line_t *line);
void disassemble(line_t *line);

// project.c

void load_settings(void);
void save_settings(void);
void backup_settings(void);
void restore_settings(void);
int get_setting(int key);
void set_setting(int key, int value);

void new_project(project_t *proj);
void close_project(project_t *proj);

void add_tab(project_t *proj);
void delete_tab(project_t *proj, int idx);
void switch_tab(project_t *proj, int idx);

// editing.c

void create_text(text_t *text);
void close_text(text_t *text);

void add_line(text_t *text, line_t *current, int above);
void clear_line(text_t *text, line_t *line, int completely);

int find_line(line_t *start, line_t *end);
line_t *walk_lines(line_t *line, int count);

void start_selection(text_t *text);
void end_selection(text_t *text);
void update_selection(text_t *text);
void delete_selection(text_t *text);

void move_cursor(int dir, int shift);
void delete_text(int back, int shift);
void insert_char(char ch);
void copy_text(int cut);
void paste_text(void);

void set_column(text_t *text, int col);
void set_row(text_t *text, line_t *row);

void process_line(void);

/*
void input_undo(void);
void input_redo(void);
void input_cursor(int x, int y);
void input_mousedown(int mouse);
void input_mouseup(int mouse);
void input_scroll(int vel);
*/

// gui.c

#include <windows.h>

enum brush_e {
	blank,
	border,
	caret,
	hl,
	white,
	grey,
	blue,
	red,
	n_brushes
};

void init_brushes(void);
HBRUSH get_brush(enum brush_e idx);
void delete_brushes(void);

HFONT get_font(int idx);

HWND get_window(int idx);
void set_window(int idx, HWND hwnd);
void refresh_window(int idx);
void set_title(const char *title);

int get_window_index(HWND hwnd);
HWND spawn_window(int ex_style, const char *class, const char *name, int style, int x, int y, int w, int h);

void update_display(int width, int height);

HWND init_gui(HINSTANCE hInstance, WNDPROC main_proc);

// display.c

LONG_PTR get_wnd_proc(int wnd);

void resize_display(int asm_x, int asm_w, int bin_x, int bin_w, int y, int h);

int window_from_coords(int x, int y);
void set_caret_from_coords(int wnd, int x, int y);

// main.c

void start_editing();
void stop_editing();

int get_focus(void);
void set_focus(int wnd);

void set_texts(text_t *asm_txt, text_t *bin_txt);
text_t *text_of(int wnd);
text_t *opposed_text(text_t *text);

// debug.c

void init_debug(void);
void debug_string(char *str);

void debug_winmsg(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void debug_wndctl(int idx);

#endif