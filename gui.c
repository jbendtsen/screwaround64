#include "header.h"

HBRUSH brush[n_brushes] = {NULL};

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

HFONT font = NULL;

HFONT get_font(int idx) {
	return font;
}

HWND window[N_WNDS] = {NULL};

// No bounds checking because I trust myself (barely)
HWND get_window(int idx) {
	return window[idx];
}
void set_window(int idx, HWND hwnd) {
	window[idx] = hwnd;
}

void refresh_window(int idx) {
	InvalidateRect(window[idx], NULL, 0);
	UpdateWindow(window[idx]);
}

void set_title(const char *title) {
	SetWindowText(window[MAIN_WND], title);
}

#define ID_OFF 32

int get_window_index(HWND hwnd) {
	// Add 1 to account for the main window
	return GetDlgCtrlID(hwnd) - ID_OFF + 1;
}

int n_ids = 0;

HWND spawn_window(int ex_style, const char *class, const char *name, int style, int x, int y, int w, int h) {
	int id = ID_OFF + n_ids;
	n_ids++;

	return CreateWindowEx(ex_style, class, name, style, x, y, w, h,
	                      window[MAIN_WND], (HMENU)id, GetModuleHandle(NULL), NULL);
}

void update_display(int width, int height) {
	int w = (width - (X_OFF * 2) - MID_GAP) / 2;
	int h = height - Y_OFF - BOTTOM;

	if (w < 0) w = 0;
	if (h < 0) h = 0;

	int w_bin = w < MAX_BIN_WIDTH ? w : MAX_BIN_WIDTH;
	int w_asm = w + (w - w_bin);
	int x_bin = X_OFF + w_asm + MID_GAP;

	resize_display(X_OFF, w_asm, x_bin, w_bin, Y_OFF, h);

	int ah_x = X_OFF + BORDER;
	int ah_w = w_asm - (2 * BORDER);
	int bh_x = x_bin + BORDER;
	int bh_w = w_bin - (2 * BORDER);

	if (ah_w < 0) ah_w = 0;
	if (bh_w < 0) bh_w = 0;

	int hh = Y_OFF - HEADING_Y;
	SetWindowPos(window[ASM_HEAD], NULL, ah_x, HEADING_Y, ah_w, hh, SWP_NOZORDER);
	SetWindowPos(window[BIN_HEAD], NULL, bh_x, HEADING_Y, bh_w, hh, SWP_NOZORDER);
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

	window[MAIN_WND] = CreateWindowEx(
		WS_EX_CLIENTEDGE,
		g_szClassName,
		NAME,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, WIDTH, HEIGHT,
		NULL, NULL, hInstance, NULL);

	if (!window[MAIN_WND]) {
		MessageBox(NULL, "Window Creation Failed!", "Error!",
			MB_ICONEXCLAMATION | MB_OK);
		return NULL;
	}

	init_brushes();

	// TO-DO: Actual font handling
	font = (HFONT)GetStockObject(SYSTEM_FIXED_FONT);

	int style = WS_CHILD | WS_VISIBLE;
	window[ASM_WND] = spawn_window(WS_EX_CLIENTEDGE, "EDIT", "", style, 0, 0, 0, 0);
	window[BIN_WND] = spawn_window(WS_EX_CLIENTEDGE, "EDIT", "", style, 0, 0, 0, 0);

	SetWindowLongPtr(window[ASM_WND], GWLP_WNDPROC, get_wnd_proc(ASM_WND));
	SetWindowLongPtr(window[BIN_WND], GWLP_WNDPROC, get_wnd_proc(BIN_WND));

	style = WS_CHILD | WS_VISIBLE | SS_LEFT;
	window[ASM_HEAD] = spawn_window(0, "STATIC", "Assembly", style, 0, 0, 0, 0);
	window[BIN_HEAD] = spawn_window(0, "STATIC", "Binary", style, 0, 0, 0, 0);

	return window[MAIN_WND];
}