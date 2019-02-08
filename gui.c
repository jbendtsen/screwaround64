#include "header.h"

#define TMR_ID_OFF 16
#define WND_ID_OFF 32

#define N_X_ELEMS 5
#define N_Y_ELEMS 8

typedef struct {
	int min_px;
	int max_px;
	float fraction;
	int hidden;
} elem_size;

elem_size x_elems[N_X_ELEMS] = {0};
elem_size y_elems[N_Y_ELEMS] = {0};

HBRUSH brush[n_brushes] = {NULL};

window_t window[N_WNDS] = {0};

void init_brushes(void) {
	brush[blank] =  CreateSolidBrush(BACKGROUND);
	brush[border] = CreateSolidBrush(RGB(0xb0, 0xd0, 0xff));
	brush[caret] =  CreateSolidBrush(RGB(0x20, 0x20, 0x20));
	brush[hl] =     CreateSolidBrush(RGB(0x20, 0x50, 0xff));
	brush[white] =  CreateSolidBrush(RGB(0xff, 0xff, 0xff));
	brush[grey] =   CreateSolidBrush(RGB(0xf0, 0xf0, 0xf0));
	brush[blue] =   CreateSolidBrush(RGB(0xd0, 0xe8, 0xff));
	brush[red] =    CreateSolidBrush(RGB(0xff, 0xd0, 0xd0));
}

HBRUSH get_brush(enum brush_e idx) {
	return brush[idx];
}

void delete_brushes(void) {
	int i;
	for (i = 0; i < n_brushes; i++)
		DeleteObject(brush[i]);
}

// No bounds checking because I trust myself (barely)
window_t *get_window(int idx) {
	return &window[idx];
}

int window_from_text(text_t *txt) {
	int i;
	for (i = 0; i < N_WNDS; i++)
		if (window[i].text == txt) return i;

	return -1;
}

int window_from_coords(int x, int y) {
	int i;
	window_t *wnd = &window[1];
	for (i = 1; i < N_WNDS; i++, wnd++) {
		if (x >= wnd->x && x < wnd->x + wnd->w &&
		    y >= wnd->y && y < wnd->y + wnd->h)
			return i;
	}

	return MAIN_WND;
}

// Checks to see if the hwnd is registered and up-to-date.
// If not, it will look up the hwnd in the local window list.
int window_from_handle(HWND hwnd) {
	int id = GetDlgCtrlID(hwnd);
	if (id)
		return id - WND_ID_OFF + 1;

	int i;
	for (i = 0; i < N_WNDS; i++)
		if (window[i].hwnd == hwnd) return i;

	return -1;
}

void refresh_window(int idx) {
	if (!window[idx].hidden) {
		InvalidateRect(window[idx].hwnd, NULL, 0);
		UpdateWindow(window[idx].hwnd);
	}
}

void hide_window(int idx, int hide) {
	window_t *wnd = &window[idx];

	if (hide)
		ShowWindow(wnd->hwnd, SW_HIDE);
	else {
		ShowWindow(wnd->hwnd, SW_RESTORE);
		UpdateWindow(wnd->hwnd);
	}

	wnd->hidden = hide;
}

void reset_caret_timer(int idx) {
	window[idx].timer = 0;
}

void tick_caret(int idx) {
	window[idx].timer++;
	if (window[idx].timer >= CARET_BLINK * 2)
		reset_caret_timer(idx);
}

void set_texts(tab_t *tab) {
	if (!tab) {
		int i;
		for (i = 0; i < N_WNDS; i++)
			window[i].text = NULL;

		return;
	}

	window[MAIN_WND].text = NULL;
	window[ASM_WND].text = &tab->asm_text;
	window[BIN_WND].text = &tab->bin_text;
	window[ERR_LIST].text = &tab->err_list;
	window[ERR_INFO].text = &tab->err_info;
	window[ASM_HEAD].text = NULL;
	window[BIN_HEAD].text = NULL;
}

text_t *text_of(int idx) {
	return window[idx].text;
}

text_t *opposed_text(text_t *txt) {
	if (txt == window[ASM_WND].text)
		return window[BIN_WND].text;
	if (txt == window[BIN_WND].text)
		return window[ASM_WND].text;

	return NULL;
}

void set_title(const char *title) {
	SetWindowText(window[MAIN_WND].hwnd, title);
}

int n_ids = 0;

HWND spawn_window(int ex_style, const char *class, const char *name, int style) {
	int id = WND_ID_OFF + n_ids;
	n_ids++;

	return CreateWindowEx(ex_style, class, name, style, 0, 0, 0, 0,
	                      window[MAIN_WND].hwnd, (HMENU)id, GetModuleHandle(NULL), NULL);
}

LRESULT CALLBACK display_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	int w = window_from_handle(hwnd);
	if (w < 0)
		return 0;

	window[w].hwnd = hwnd; // In case it's a new handle
	if (w == ASM_WND || w == BIN_WND)
		return code_display(w, uMsg, wParam, lParam);
	else if (w == ERR_LIST || w == ERR_INFO)
		return error_display(w, uMsg, wParam, lParam);

	return 0;
}

VOID CALLBACK timer_proc(HWND hwnd, UINT uMsg, UINT_PTR idTimer, DWORD dwTime) {
	int w = idTimer - TMR_ID_OFF;
	window[w].hwnd = hwnd;

	if (w == get_focus()) {
		tick_caret(w);
		refresh_window(w);
	}
}

void init_display_window(int idx) {
	// Create the window
	int style = WS_CHILD | WS_VISIBLE;
	window[idx].hwnd = spawn_window(WS_EX_CLIENTEDGE, "EDIT", "", style);

	// Subclass it with the given window process
	SetWindowLongPtr(window[idx].hwnd, GWLP_WNDPROC, (LONG_PTR)display_proc);

	// Give the window our caret-blinking timer process
	int tm_id = TMR_ID_OFF + idx;
	SetTimer(window[idx].hwnd, tm_id, TIMER_TICK, timer_proc);

	// Give it the stock font (for now)
	window[idx].font = (HFONT)GetStockObject(SYSTEM_FIXED_FONT);
}

void init_heading(int idx, const char *title) {
	int style = WS_CHILD | WS_VISIBLE | SS_LEFT;
	window[idx].hwnd = spawn_window(0, "STATIC", title, style);
}

HWND init_gui(HINSTANCE hInstance, WNDPROC main_proc) {
	char *g_szClassName = "SC64";

	WNDCLASSEX wc;
	wc.cbSize        = sizeof(WNDCLASSEX);
	wc.style         = CS_GLOBALCLASS | CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc   = main_proc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = hInstance;
	wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = CreateSolidBrush(BACKGROUND);
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = g_szClassName;
	wc.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);

	ATOM main_class = RegisterClassEx(&wc);
	if (!main_class) {
		MessageBox(NULL, "Window Registration Failed!", "Error!",
			MB_ICONEXCLAMATION | MB_OK);
		return NULL;
	}

	window[MAIN_WND].hwnd = CreateWindowEx(
		WS_EX_CLIENTEDGE,
		g_szClassName,
		NAME,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, WIDTH, HEIGHT,
		NULL, NULL, hInstance, NULL);

	if (!window[MAIN_WND].hwnd) {
		MessageBox(NULL, "Window Creation Failed!", "Error!",
			MB_ICONEXCLAMATION | MB_OK);
		return NULL;
	}

	x_elems[0] = (elem_size){.max_px = 20, .fraction = 0.05}; // left gap
	x_elems[1] = (elem_size){.fraction = 0.45}; // ASM window
	x_elems[2] = (elem_size){.max_px = 20, .fraction = 0.05}; // mid gap
	x_elems[3] = (elem_size){.max_px = 200, .fraction = 0.40}; // BIN window
	x_elems[4] = x_elems[0]; // right gap

	y_elems[0] = (elem_size){.max_px = 10, .fraction = 0.01}; // top gap
	y_elems[1] = (elem_size){.min_px = 20, .max_px = 30, .fraction = 0.05}; // headings
	y_elems[2] = (elem_size){.fraction = 0.6}; // code windows
	y_elems[3] = (elem_size){.max_px = 15, .fraction = 0.01}; // code gap
	y_elems[4] = (elem_size){.max_px = 100, .fraction = 0.20, .hidden = 0}; // error list window
	y_elems[5] = (elem_size){.max_px = 10, .fraction = 0.01, .hidden = 0}; // error gap
	y_elems[6] = (elem_size){.max_px = 50, .fraction = 0.10, .hidden = 0}; // error info window
	y_elems[7] = (elem_size){.max_px = 20, .fraction = 0.02}; // bottom gap

	init_brushes();

	init_display_window(ASM_WND);
	init_display_window(BIN_WND);
	init_display_window(ERR_LIST);
	init_display_window(ERR_INFO);

	init_heading(ASM_HEAD, " Assembly");
	init_heading(BIN_HEAD, " Binary");

	return window[MAIN_WND].hwnd;
}

void compute_offsets(int *offsets, int scale, elem_size *elems, int n_elems) {
	offsets[0] = 0;
	int n_no_max = 0, n_min = 0;

	int i, j;
	for (i = 0; i < n_elems; i++) {
		elem_size *e = &elems[i];
		if (e->hidden) {
			offsets[i+1] = offsets[i];
			continue;
		}

		if (e->min_px > 0)
			n_min++;
		if (e->max_px == 0)
			n_no_max++;

		int sz = (float)scale * e->fraction;
		sz = sz < e->min_px ? e->min_px : sz;

		if (e->max_px > 0)
			sz = sz > e->max_px ? e->max_px : sz;

		offsets[i+1] = offsets[i] + sz;
	}

	int diff = scale - offsets[n_elems];
	if (diff > 0) {
		int boost = diff / n_no_max;
		for (i = 0; i < n_elems; i++) {
			if (elems[i].max_px == 0 && !elems[i].hidden) {
				for (j = i; j < n_elems; j++)
					offsets[j+1] += boost;
			}
		}
	}
	else if (diff < 0) {
		int nerf = diff / n_min;
		for (i = 0; i < n_elems; i++) {
			if (elems[i].min_px > 0 && !elems[i].hidden) {
				for (j = i; j < n_elems; j++)
					offsets[j+1] -= nerf;
			}
		}
	}
}

void repos_window(int idx, int *elem_x, int leap_x, int *elem_y, int leap_y) {
	window_t *wnd = &window[idx];

	wnd->x = *elem_x;
	wnd->y = *elem_y;
	wnd->w = *(elem_x + leap_x) - *elem_x;
	wnd->h = *(elem_y + leap_y) - *elem_y;

	SetWindowPos(wnd->hwnd, NULL, wnd->x, wnd->y, wnd->w, wnd->h, SWP_NOZORDER);
}

void resize_subwindows(int width, int height) {
	int x_pos[N_X_ELEMS + 1];
	int y_pos[N_Y_ELEMS + 1];

	compute_offsets(&x_pos[0], width, &x_elems[0], N_X_ELEMS);
	compute_offsets(&y_pos[0], height, &y_elems[0], N_Y_ELEMS);

	repos_window(ASM_WND, &x_pos[1], 1, &y_pos[2], 1);
	repos_window(BIN_WND, &x_pos[3], 1, &y_pos[2], 1);
	repos_window(ERR_LIST, &x_pos[1], 3, &y_pos[4], 1);
	repos_window(ERR_INFO, &x_pos[1], 3, &y_pos[6], 1);
	repos_window(ASM_HEAD, &x_pos[1], 1, &y_pos[1], 1);
	repos_window(BIN_HEAD, &x_pos[3], 1, &y_pos[1], 1);
}