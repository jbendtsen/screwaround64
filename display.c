#include "header.h"

int disp_init = 0;

int cur_wnd = 0;

int asm_x = 0, asm_w = 0;
int bin_x = 0, bin_w = 0;
int wnd_y = 0, wnd_h = 0;

int lspace = 0;
int lheight = 0;

int update_display(UINT uMsg, WPARAM wParam, LPARAM lParam);

int invoke_display(int wnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	cur_wnd = wnd;
	return update_display(uMsg, wParam, lParam);
}

void resize_display(int x1, int w1, int x2, int w2, int y, int h) {
	asm_x = x1;
	asm_w = w1;
	bin_x = x2;
	bin_w = w2;
	wnd_y = y;
	wnd_h = h;

	SetWindowPos(get_window(ASM_WND), NULL, asm_x, wnd_y, asm_w, wnd_h, SWP_NOZORDER);
	SetWindowPos(get_window(BIN_WND), NULL, bin_x, wnd_y, bin_w, wnd_h, SWP_NOZORDER);
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

	if (!hdc || !str || !strlen(str) || col == 0)
		return offset;

	int len = strlen(str);
	col = (col >= 0 && col < len) ? col : len;

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
		if (top + (c + 1) * lheight > y)
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

	HWND hwnd = get_window(wnd);
	HDC hdc = GetDC(hwnd);

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

 	ReleaseDC(hwnd, hdc);
}

struct sc_point {
	HDC hdc;
	int x, y;
};

void render_text(struct sc_point *disp, char *str, int len) {
	if (len <= 0) len = strlen(str);
	TextOut(disp->hdc, disp->x, disp->y, str, len);
}

void draw_display(int wnd, HDC hdc) {
	text_t *text = text_of(wnd);
	if (!text || !hdc)
		return;

	// Determine the colour palatte to use
	HBRUSH outline, back;
	int focus = get_focus();
	if (wnd == focus) {
		outline = get_brush(border);
		back = get_brush(white);
	}
	else {
		outline = get_brush(blank);
		back = get_brush(grey);
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

	int multi_sel = 0;
	int y = 0;

	HFONT old_font = (HFONT)SelectObject(hdc, get_font(DISP_FONT));

	while (line && y < wnd_h - ((BORDER * 2) + lheight)) {
		if (line == a && text->sel > 1) {
			def = SetTextColor(hdc, WHITE);
			multi_sel = 1;
		}

		if (wnd == focus && line == text->cur) {
			HBRUSH colour = is_editing() ? get_brush(blue) : get_brush(grey);
			highlight_line(wnd, hdc, colour, y);
		}

		int top = BORDER + y;
		int left = BORDER + MARGIN;
		struct sc_point disp = {hdc, left, top + lspace};

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

				fill_box(hdc, get_brush(hl), a_px, top, b_px, top + lheight);

				if (a_pos > 0)
					render_text(&disp, line->str, a_pos);

				def = SetTextColor(hdc, WHITE);

				disp.x = a_px;
				render_text(&disp, line->str + a_pos, b_pos - a_pos);

				SetTextColor(hdc, def);

				if (b_pos < len) {
					disp.x = b_px;
					render_text(&disp, line->str + b_pos, 0);
				}
			}
			else {
				if (multi_sel) {
					int x = left - MARGIN;
					fill_box(hdc, get_brush(hl), x, top, width - x, top + lheight);
				}
				render_text(&disp, line->str, 0);
			}
		}

		if (wnd == focus) {
			if (line == text->cur) {
				if (get_caret_timer(wnd) < CARET_BLINK) {
					int x = calc_column_distance(hdc, line->str, line->col) - 1;
					fill_box(hdc, get_brush(caret), x, top, x + CARET, top + lheight);
				}
			}

			if (line == b && text->sel > 1) {
				SetTextColor(hdc, def);
				multi_sel = 0;
			}
		}

		y += lheight;
		line = line->next;
	}

	SelectObject(hdc, old_font);
}

HCURSOR ibeam = NULL;

int update_display(UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if (uMsg == WM_GETDLGCODE)
		return DLGC_WANTALLKEYS;

	if (uMsg == WM_ERASEBKGND)
		return 1;

	if (uMsg == WM_PAINT) {
		PAINTSTRUCT ps;
		HWND hwnd = get_window(cur_wnd);
		HDC hdc = BeginPaint(hwnd, &ps);

		SetMapMode(hdc, MM_TEXT);
		SetBkMode(hdc, TRANSPARENT);

		if (!lheight) {
			TEXTMETRIC tm = {0};
			GetTextMetrics(hdc, &tm);

			// A 'point' is a unit of measurement equivalent to 1/72th of an inch.
			// The value returned here will depend on the display configuration.
			float pixels_per_point = (float)GetDeviceCaps(hdc, LOGPIXELSY) / 72.0;

			/*
				To convert our height from LUs to pixels, we multiply it by the number of pixels per LU,
				which is broken into 2 steps to reduce loss from integer truncation:
			*/
			lspace = tm.tmInternalLeading + tm.tmExternalLeading;
			lspace = (float)lspace * pixels_per_point / 2.0;

			lheight = tm.tmHeight + tm.tmExternalLeading;
			lheight = (float)lheight * pixels_per_point;
		}

		draw_display(cur_wnd, hdc);
		EndPaint(hwnd, &ps);
	}

	if (uMsg == WM_LBUTTONDOWN) {
		int x = lParam & 0xffff;
		int y = lParam >> 16;

		int width = cur_wnd == ASM_WND ? asm_w : bin_w;
		if (!(x >= BORDER && x < width - BORDER && y >= BORDER && y < wnd_h - BORDER))
			return 0;

		set_caret_from_coords(cur_wnd, x, y);
		set_focus(cur_wnd);
		refresh_window(cur_wnd);
	}

	if (uMsg == WM_SETCURSOR) {
		if (!ibeam)
			ibeam = LoadCursor(NULL, IDC_IBEAM);

		SetCursor(ibeam);
	}

	if (uMsg == WM_NCHITTEST)
		return HTTRANSPARENT;

	return 0;
}