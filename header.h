#ifndef SC64_H
#define SC64_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NAME "Screwaround64"

#define MAIN_WND 0
#define ASM_WND  1
#define BIN_WND  2
#define ERR_LIST 3
#define ERR_INFO 4
#define ASM_HEAD 5
#define BIN_HEAD 6

#define N_WNDS   7

#define WIDTH  600
#define HEIGHT 550

// In pixels (px)
#define X_OFF   20
#define Y_OFF   35
#define MID_GAP 20
#define BOTTOM  20
#define HEADING_Y (Y_OFF - 23)

// In percentage of height (vh)
#define CODE_H 70
#define ERR_GAP 5
#define ERR_H  25

#define MAX_BIN_WIDTH 300

#define BORDER 2
#define MARGIN 3
#define CARET  2

#define BLACK      RGB(0x00, 0x00, 0x00)
#define WHITE      RGB(0xff, 0xff, 0xff)
#define BACKGROUND RGB(0xe0, 0xe0, 0xe0)

#define CARET_BLINK 10
#define TIMER_TICK 100

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

struct text_t {
	int type; // How the text is used/rendered

	line_t *first, *last; // First and last lines in the list
	line_t *top;          // First and last VISIBLE lines
	line_t *cur;          // Current line
	line_t *start, *end;  // Currently selected lines, from 'start' to 'end'

	int column;
	int sel; // Selection type: 0 = no selection, 1 = single line, 2 = start before end, 3 = end before start

	// markings/comment information go here...
};
typedef struct text_t text_t;

typedef struct {
	char *title;
	char *desc;
	u32 ram;
	u32 rom;

	text_t asm_text; // not pointers
	text_t bin_text;
	text_t err_list;
	text_t err_info;
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

#include <windows.h>

typedef struct {
	HWND hwnd;
	HFONT font;

	text_t *text;
	int lspace;
	int lheight;

	int x, y, w, h;
	int timer;
	int hidden;
} window_t;

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

void create_texts(tab_t *tab);
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

/*
void input_undo(void);
void input_redo(void);
*/

// gui.c

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

window_t *get_window(int idx);
int window_from_text(text_t *txt);
int window_from_coords(int x, int y);
int window_from_handle(HWND hwnd);

void hide_window(int idx, int hide);
void refresh_window(int idx);

void reset_caret_timer(int idx);
void tick_caret(int idx);

void set_texts(tab_t *tab);
text_t *text_of(int idx);
text_t *opposed_text(text_t *text);

HWND spawn_window(int ex_style, const char *class, const char *name, int style);

void set_title(const char *title);
void resize_subwindows(int width, int height);

HWND init_gui(HINSTANCE hInstance, WNDPROC main_proc);

// display.c

int calc_visible_lines(int idx, line_t *top);
void set_caret_from_coords(int idx, int x, int y);

int code_display(int idx, UINT uMsg, WPARAM wParam, LPARAM lParam);
int error_display(int idx, UINT uMsg, WPARAM wParam, LPARAM lParam);

// main.c

void start_editing(void);
void stop_editing(void);
int is_editing(void);

int get_focus(void);
void set_focus(int wnd);
text_t *focussed_text(void);

void process_line(void);

// debug.c

void init_debug(void);
void debug_string(char *str);

void debug_winmsg(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void debug_xy(int x, int y);

#endif