#include "header.h"

int disp_init = 0;

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
} brushes;

HBRUSH brush[n_brushes] = {NULL};
int brush_init = 0;

HFONT font = NULL;

HWND window[] = {NULL, NULL, NULL};
int cur_wnd = 0;

int asm_x = 0, asm_w = 0;
int bin_x = 0, bin_w = 0;
int wnd_y = 0, wnd_h = 0;

int lheight = 0;

void refresh() {
	InvalidateRect(window[ASM_WND], NULL, 0);
	InvalidateRect(window[BIN_WND], NULL, 0);
	UpdateWindow(window[ASM_WND]);
	UpdateWindow(window[BIN_WND]);
}

LRESULT edit_proc(UINT uMsg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK asm_edit_func(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	cur_wnd = ASM_WND;
	window[cur_wnd] = hwnd;
	return edit_proc(uMsg, wParam, lParam);
}

LRESULT CALLBACK bin_edit_func(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	cur_wnd = BIN_WND;
	window[cur_wnd] = hwnd;
	return edit_proc(uMsg, wParam, lParam);
}

void resize_display(int x1, int w1, int x2, int w2, int y, int h) {
	if (!disp_init) {
		int style = WS_CHILD | WS_VISIBLE | ES_MULTILINE | DS_LOCALEDIT;

		window[ASM_WND] = spawn_window(WS_EX_CLIENTEDGE, "EDIT", "", style, x1, y, w1, h);
		window[BIN_WND] = spawn_window(WS_EX_CLIENTEDGE, "EDIT", "", style, x2, y, w2, h);

		// TO-DO: Actual font handling
		font = (HFONT)GetStockObject(SYSTEM_FIXED_FONT);

		SetWindowLongPtr(window[ASM_WND], GWLP_WNDPROC, (LONG_PTR)asm_edit_func);
		SetWindowLongPtr(window[BIN_WND], GWLP_WNDPROC, (LONG_PTR)bin_edit_func);

		disp_init = 1;
	}
	else {
		SetWindowPos(window[ASM_WND], NULL, x1, y, w1, h, SWP_NOZORDER);
		SetWindowPos(window[BIN_WND], NULL, x2, y, w2, h, SWP_NOZORDER);
	}

	asm_x = x1;
	asm_w = w1;
	bin_x = x2;
	bin_w = w2;
	wnd_y = y;
	wnd_h = h;
}

int window_from_coords(int x, int y) {
	if (y < wnd_y || y >= wnd_y + wnd_h)
		return MAIN_WND;

	if (x >= asm_x && x < asm_x + asm_w)
		return ASM_WND;
	if (x >= bin_x && x < bin_x + bin_w)
		return BIN_WND;

	return MAIN_WND;
}

void fill_box(HDC hdc, HBRUSH br, int left, int top, int right, int bottom) {
	RECT r;
	r.left = left;
	r.top = top;
	r.right = right;
	r.bottom = bottom;

	FillRect(hdc, &r, br);
}

void highlight_line(int wnd, HDC hdc, HBRUSH brush, int y) {
	if (!hdc || !brush || y < 0 || y > wnd_h - lheight)
		return;

	int width = wnd == ASM_WND ? asm_w : bin_w;

	RECT r;
	r.left = BORDER;
	r.top = BORDER + y;
	r.right = width - BORDER;
	r.bottom = BORDER + y + lheight;
	FillRect(hdc, &r, brush);
}

int calc_column_distance(HDC hdc, char *str, int col) {
	int offset = BORDER + MARGIN;

	if (!hdc || !str || !strlen(str) || col <= 0)
		return offset;

	col = col < strlen(str) ? col : strlen(str);

	RECT r = {0};
	DrawText(hdc, str, col, &r, DT_CALCRECT);
	return offset + r.right;
}

int calc_visible_lines(line_t *top) {
	if (!top)
		return 0;

	int n = 0;
	line_t *line = top;
	while (line && (n * lheight) < wnd_h - ((BORDER * 2) + lheight)) {
		n++;
		line = line->next;
	}

	return n;
}

void set_caret_from_coords(int wnd, int x, int y) {
	if (wnd != ASM_WND && wnd != BIN_WND)
		return;

	int x_wnd, w_wnd;
	if (wnd == ASM_WND) {
		x_wnd = asm_x;
		w_wnd = asm_w;
	}
	else {
		x_wnd = bin_x;
		w_wnd = bin_w;
	}

	x -= x_wnd;
	y -= wnd_y;

	if (x < 0 || x >= w_wnd || y < 0 || y >= wnd_h)
		return;

	// Find the row

	text_t *text = text_of(wnd);
	line_t *line = text->top;
	int c = 0, top = BORDER;

	while (line && (c * lheight) < wnd_h - (top + lheight + BORDER)) {
		if (top + (c + 1) * wnd_h > y)
			break;

		c++;
		line = line->next;
	}
	if (!line)
		line = text->last;

	set_row(text, line);

	// Find the column

	x -= BORDER + MARGIN;
	if (x < 0 || !line->str || !strlen(line->str)) {
		line->col = 0;
		return;
	}

	HDC hdc = GetDC(window[wnd]);
	RECT r = {0};
	int i, prev, next = 0;
	int beyond = 1;

	for (i = 0; i < strlen(line->str); i++) {
		DrawText(hdc, line->str, i+1, &r, DT_CALCRECT);
		prev = next;
		next = r.right;

		if (next > x) {
/*
			char buf[64];
			sprintf(buf, "x: %d, prev: %d, next: %d", x, prev, next);
			debug_string(buf);
*/

			if (x - prev > next - x - 1)
				line->col = i+1;
			else
				line->col = i;

			beyond = 0;
			break;
		}
	}
	if (beyond)
		line->col = strlen(line->str);

	ReleaseDC(window[wnd], hdc);
}

void draw_display(int wnd, HDC hdc) {
	text_t *text = text_of(wnd);
	if (!text || !hdc)
		return;

	if (!brush_init) {
		brush[blank] =  CreateSolidBrush(BACKGROUND);
		brush[border] = CreateSolidBrush(RGB(0xb0, 0xd0, 0xff));
		brush[caret] =  CreateSolidBrush(RGB(0x20, 0x20, 0x20));
		brush[hl] =     CreateSolidBrush(RGB(0x20, 0x50, 0xff));
		brush[white] =  CreateSolidBrush(RGB(0xff, 0xff, 0xff));
		brush[grey] =   CreateSolidBrush(RGB(0xf0, 0xf0, 0xf0));
		brush[blue] =   CreateSolidBrush(RGB(0xd0, 0xe8, 0xff));
		brush[red] =    CreateSolidBrush(RGB(0xff, 0xd0, 0xd0));

		brush_init = 1;
	}

	// Determine the colour palatte to use
	HBRUSH outline, back;
	int focus = get_focus();
	if (wnd == focus) {
		outline = brush[border];
		back = brush[white];
	}
	else {
		outline = brush[blank];
		back = brush[grey];
	}

	int width = wnd == ASM_WND ? asm_w : bin_w;

	// Paint the border
	fill_box(hdc, outline, 0, 0, width, BORDER); // the whole top
	fill_box(hdc, outline, 0, wnd_h - BORDER, width, wnd_h); // the whole bottom
	fill_box(hdc, outline, 0, BORDER, BORDER, wnd_h - BORDER); // the remainder on the left
	fill_box(hdc, outline, width - BORDER, BORDER, width, wnd_h - BORDER); // the remainder on the right

	// Paint the background
	fill_box(hdc, back, BORDER, BORDER, width - BORDER, wnd_h - BORDER);

	// Get the start and end points for the current selection (if there is one)
	// If the end point is before the start point, swap our copies of the points
	line_t *a = text->start;
	line_t *b = text->end;
	if (text->sel == 3) {
		line_t *temp = a;
		a = b;
		b = temp;
	}

	COLORREF def = 0;
	line_t *line = text->top;
	int cur = -1;
	int sel = 0;
	int y = 0;

	HFONT old_font = (HFONT)SelectObject(hdc, font);

	while (line && y < wnd_h - ((BORDER * 2) + lheight)) {
		if (line == a && text->sel > 1) {
			def = SetTextColor(hdc, WHITE);
			sel = 1;
		}

		if (wnd == focus && line == text->cur) {
			highlight_line(wnd, hdc, brush[blue], y);
			cur = line->col;
		}

		int top = BORDER + y;
		int left = BORDER + MARGIN;

		top += 2; // HACK!

		if (line->str) {
			int len = strlen(line->str);

			if (text->sel == 1 && line == text->cur && line->start != line->end) {
				int a_pos = line->start;
				int b_pos = line->end;

				if (a_pos > b_pos) {
					int temp = a_pos;
					a_pos = b_pos;
					b_pos = temp;
				}
				if (a_pos < 0) a_pos = 0;
				if (b_pos > len) b_pos = len;

				int a_px = calc_column_distance(hdc, line->str, a_pos);
				int b_px = calc_column_distance(hdc, line->str, b_pos);

				fill_box(hdc, brush[hl], a_px, top, b_px, top + lheight);

				if (a_pos)
					TextOut(hdc, left, top, line->str, a_pos);

				def = SetTextColor(hdc, WHITE);
				TextOut(hdc, a_px, top, line->str + a_pos, b_pos - a_pos);
				SetTextColor(hdc, def);

				if (b_pos < len)
					TextOut(hdc, b_px, top, line->str + b_pos, len - b_pos);
			}
			else {
				if (sel) {
					RECT r = {0};
					DrawText(hdc, line->str, -1, &r, DT_CALCRECT);
					fill_box(hdc, brush[hl], left, top, left + r.right, top + lheight);
				}
				TextOut(hdc, left, top, line->str, len);
			}
		}

		if (cur >= 0) {
			RECT r = {0};
			if (line->str)
				DrawText(hdc, line->str, line->col, &r, DT_CALCRECT);

			left += r.right - 1;

			fill_box(hdc, brush[caret], left, top, left + CARET, top + lheight);
			cur = -1;
		}

		if (line == b && text->sel > 1) {
			SetTextColor(hdc, def);
			sel = 0;
		}

		y += lheight;
		line = line->next;
	}

	SelectObject(hdc, old_font);
}

HCURSOR ibeam = NULL;

LRESULT edit_proc(UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if (uMsg == WM_GETDLGCODE)
		return DLGC_WANTALLKEYS;

	if (uMsg == WM_ERASEBKGND)
		return 1;

	if (uMsg == WM_PAINT) {
		HDC hdc;
		PAINTSTRUCT ps;

		hdc = BeginPaint(window[cur_wnd], &ps);
		SetMapMode(hdc, MM_TEXT);
		SetBkMode(hdc, TRANSPARENT);

		if (!lheight) {
			TEXTMETRIC tm = {0};
			GetTextMetrics(hdc, &tm);

			int pixels_per_inch = GetDeviceCaps(hdc, LOGPIXELSY); // varies with different displays

			/*
				To convert our height from LUs to pixels, we multiply it by the number of pixels per LU,
				which is broken into 2 steps to reduce loss from integer truncation:
			*/
			lheight = tm.tmHeight + tm.tmExternalLeading; // total height in Logical Units (1 LU = 1/72 inch)
			lheight *= pixels_per_inch;
			lheight /= 72; // divide by 72 to get the final answer
		}

		draw_display(cur_wnd, hdc);
		EndPaint(window[cur_wnd], &ps);
	}

	if (uMsg == WM_LBUTTONDOWN) {
		int x = lParam & 0xffff;
		int y = lParam >> 16;

		int width = cur_wnd == ASM_WND ? asm_w : bin_w;
		if (!(x >= BORDER && x < width - BORDER && y >= BORDER && y < wnd_h - BORDER))
			return 0;

		set_caret_from_coords(cur_wnd, x, y);
		set_focus(cur_wnd);
		refresh();
	}

	if (uMsg == WM_SETCURSOR) {
		if (!ibeam)
			ibeam = LoadCursor(NULL, IDC_IBEAM);

		SetCursor(ibeam);
	}

	if (uMsg == WM_NCHITTEST)
		return HTTRANSPARENT;

	if (uMsg == WM_CLOSE) {
		int i;
		for (i = 0; i < n_brushes; i++)
			DeleteObject(brush[i]);

		brush_init = 0;
	}

	return 0;
}