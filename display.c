#include "header.h"

void fill_box(HDC hdc, HBRUSH br, int left, int top, int right, int bottom) {
	RECT r;
	r.left = left;
	r.top = top;
	r.right = right;
	r.bottom = bottom;

	FillRect(hdc, &r, br);
}

void highlight_line(int idx, HDC hdc, HBRUSH brush, int y) {
	window_t *wnd = get_window(idx);
	if (!hdc || !brush || y < 0 || y > wnd->h - wnd->lheight)
		return;

	RECT r;
	r.left = BORDER;
	r.top = BORDER + y;
	r.right = wnd->w - BORDER;
	r.bottom = BORDER + y + wnd->lheight;
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

int calc_visible_lines(int idx, line_t *top) {
	if (!top)
		return 0;

	window_t *wnd = get_window(idx);

	int n = 0;
	line_t *line = top;

	while (line && (n * wnd->lheight) < wnd->h - ((BORDER * 2) + wnd->lheight)) {
		n++;
		line = line->next;
	}

	return n;
}

void set_caret_from_coords(int idx, int x, int y) {
	window_t *wnd = get_window(idx);
	if (!wnd->text)
		return;

	x -= wnd->x;
	y -= wnd->y;

	if (x < 0 || x >= wnd->w || y < 0 || y >= wnd->h)
		return;

	// Find the row

	line_t *line = wnd->text->top;
	int c = 0, top = BORDER;

	while (line && (c * wnd->lheight) < wnd->h - (top + wnd->lheight + BORDER)) {
		if (top + (c + 1) * wnd->lheight > y)
			break;

		c++;
		line = line->next;
	}
	if (!line)
		line = wnd->text->last;

	set_row(wnd->text, line);

	// Find the column

	x -= BORDER + MARGIN;
	if (x < 0 || !line->str || !strlen(line->str)) {
		line->col = 0;
		return;
	}

	HDC hdc = GetDC(wnd->hwnd);

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

 	ReleaseDC(wnd->hwnd, hdc);
}

struct sc_point {
	HDC hdc;
	int x, y;
};

void render_text(struct sc_point *disp, char *str, int len) {
	if (len <= 0) len = strlen(str);
	TextOut(disp->hdc, disp->x, disp->y, str, len);
}

void draw_edit_window(int idx, HDC hdc) {
	window_t *wnd = get_window(idx);
	text_t *text = wnd->text;
	if (!text || !hdc)
		return;

	// Determine the colour palatte to use
	HBRUSH outline, back;
	int focus = get_focus();
	if (idx == focus) {
		outline = get_brush(border);
		back = get_brush(white);
	}
	else {
		outline = get_brush(blank);
		back = get_brush(grey);
	}

	// Paint the border
	fill_box(hdc, outline, 0, 0, wnd->w, BORDER); // the whole top
	fill_box(hdc, outline, 0, wnd->h - BORDER, wnd->w, wnd->h); // the whole bottom
	fill_box(hdc, outline, 0, BORDER, BORDER, wnd->h - BORDER); // the remainder on the left
	fill_box(hdc, outline, wnd->w - BORDER, BORDER, wnd->w, wnd->h - BORDER); // the remainder on the right

	// Paint the background
	fill_box(hdc, back, BORDER, BORDER, wnd->w - BORDER, wnd->h - BORDER);

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

	HFONT old_font = (HFONT)SelectObject(hdc, wnd->font);

	while (line && y < wnd->h - ((BORDER * 2) + wnd->lheight)) {
		if (line == a && text->sel > 1) {
			def = SetTextColor(hdc, WHITE);
			multi_sel = 1;
		}

		if (idx == focus && line == text->cur) {
			HBRUSH colour = is_editing() ? get_brush(blue) : get_brush(grey);
			highlight_line(idx, hdc, colour, y);
		}

		int top = BORDER + y;
		int left = BORDER + MARGIN;
		struct sc_point disp = {hdc, left, top + wnd->lspace};

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

				fill_box(hdc, get_brush(hl), a_px, top, b_px, top + wnd->lheight);

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
					fill_box(hdc, get_brush(hl), x, top, wnd->w - x, top + wnd->lheight);
				}
				render_text(&disp, line->str, 0);
			}
		}

		if (idx == focus) {
			if (line == text->cur) {
				if (wnd->timer < CARET_BLINK) {
					int x = calc_column_distance(hdc, line->str, line->col) - 1;
					fill_box(hdc, get_brush(caret), x, top, x + CARET, top + wnd->lheight);
				}
			}

			if (line == b && text->sel > 1) {
				SetTextColor(hdc, def);
				multi_sel = 0;
			}
		}

		y += wnd->lheight;
		line = line->next;
	}

	SelectObject(hdc, old_font);
}

void get_font_height(window_t *wnd, HDC hdc) {
	TEXTMETRIC tm = {0};
	GetTextMetrics(hdc, &tm);

	// A 'point' is a unit of measurement equivalent to 1/72th of an inch.
	// The value returned here will depend on the display configuration.
	float pixels_per_point = (float)GetDeviceCaps(hdc, LOGPIXELSY) / 72.0;

	/*
		To convert our height from LUs to pixels, we multiply it by the number of pixels per LU,
		which is broken into 2 steps to reduce loss from integer truncation:
	*/
	wnd->lspace = tm.tmInternalLeading + tm.tmExternalLeading;
	wnd->lspace = (float)wnd->lspace * pixels_per_point / 2.0;

	wnd->lheight = tm.tmHeight + tm.tmExternalLeading;
	wnd->lheight = (float)wnd->lheight * pixels_per_point;
}

HCURSOR ibeam = NULL;

int error_display(int idx, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	return code_display(idx, uMsg, wParam, lParam);
}

int code_display(int idx, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if (uMsg == WM_GETDLGCODE)
		return DLGC_WANTALLKEYS;

	if (uMsg == WM_ERASEBKGND)
		return 1;

	if (uMsg == WM_PAINT) {
		PAINTSTRUCT ps;
		window_t *wnd = get_window(idx);
		HDC hdc = BeginPaint(wnd->hwnd, &ps);

		SetMapMode(hdc, MM_TEXT);
		SetBkMode(hdc, TRANSPARENT);

		if (!wnd->lheight)
			get_font_height(wnd, hdc);

		draw_edit_window(idx, hdc);
		EndPaint(wnd->hwnd, &ps);
	}

	if (uMsg == WM_LBUTTONDOWN) {
		int x = lParam & 0xffff;
		int y = lParam >> 16;

		window_t *wnd = get_window(idx);
		if (!(x >= BORDER && x < wnd->w - BORDER && y >= BORDER && y < wnd->h - BORDER))
			return 0;

		set_caret_from_coords(idx, x, y);
		set_focus(idx);
		refresh_window(idx);
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