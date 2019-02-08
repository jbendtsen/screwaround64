#include "header.h"

#define CTRL  1
#define SHIFT 2

project_t ctx;

int focus = 0;
//int hover = 0;

int editing = 0;

void start_editing() {
	set_title("* Screwaround64");
	editing = 1;
}
void stop_editing() {
	set_title("Screwaround64");
	refresh_window(ASM_WND);
	refresh_window(BIN_WND);
	editing = 0;
}
int is_editing() {
	return editing;
}

int get_focus() {
	return focus;
}
text_t *focussed_text(void) {
	return text_of(focus);
}

void set_focus(int wnd) {
	if (focus != wnd)
		stop_editing();

	int old_focus = focus;
	focus = wnd;

	reset_caret_timer(focus);
	refresh_window(old_focus);
}

void process_line() {
	text_t *txt = focussed_text();
	if (focus == ASM_WND)
		assemble(txt->cur);
	else if (focus == BIN_WND)
		disassemble(txt->cur);

	stop_editing();
}

int key_mod = 0;
int lb_held = 0;

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	get_window(MAIN_WND)->hwnd = hwnd;

	switch(uMsg) {
		case WM_CREATE:
		{
			init_debug();

			new_project(&ctx);

			resize_subwindows(WIDTH, HEIGHT);

			focus = ASM_WND;
			break;
		}
		case WM_SIZE:
		{
			RECT rect;

			GetClientRect(hwnd, &rect);
			int width = rect.right;
			int height = rect.bottom;

			resize_subwindows(width, height);
			break;
		}
		case WM_CTLCOLORSTATIC:
		{
			int idx = window_from_handle((HWND)lParam);
			if (idx != ASM_HEAD && idx != BIN_HEAD)
				break;

			SetBkMode((HDC)wParam, TRANSPARENT);
			return (INT_PTR)get_brush(blank);
		}
		case WM_KEYDOWN:
		{
			switch (wParam) {
				case VK_CONTROL:
					key_mod |= CTRL;
					break;
				case VK_SHIFT:
					key_mod |= SHIFT;
					break;

				case VK_ESCAPE:
					if (is_editing())
						process_line();
					else
						set_focus(0);
					break;

				case VK_TAB:
					if (focus == ASM_WND)
						set_focus(BIN_WND);
					else
						set_focus(ASM_WND);
					break;
			}

			if (!focus)
				break;

			switch (wParam) {

				case VK_LEFT:
				case VK_UP:
				case VK_RIGHT:
				case VK_DOWN:
				{
					if (key_mod == CTRL) {
						int dir = (wParam == VK_LEFT || wParam == VK_DOWN) ? -1 : 1;
						switch_tab(&ctx, ctx.idx + dir);
						break;
					}

					move_cursor(wParam - VK_LEFT, key_mod & SHIFT);
					break;
				}

				case VK_HOME:
					set_column(focussed_text(), 0);
					break;
				case VK_END:
					set_column(focussed_text(), -1);
					break;

				case VK_DELETE:
					start_editing();
					delete_text(0, key_mod & SHIFT);
					break;
				case VK_BACK:
					start_editing();
					delete_text(1, key_mod & SHIFT);
					break;
				case VK_RETURN:
				{
					//start_editing();
					text_t *txt = focussed_text();
					int above = (key_mod & SHIFT) | (txt->cur->col == 0);

					add_line(txt, NULL, above);
					set_row(txt, txt->cur->next);
					break;
				}
			}

			if (key_mod & CTRL && wParam >= ' ' && wParam <= '~') {
				switch (wParam) {
					case 'c': // copy
					case 'C':
						copy_text(0);
						break;
					case 'x': // cut
					case 'X':
						start_editing();
						copy_text(1);
						break;
					case 'v': // paste
					case 'V':
						// start ending based on result of paste_text()
						paste_text();
						stop_editing();
						break;
					case 'a': // select all
					case 'A':
					{
						text_t *txt = focussed_text();

						txt->start = txt->first;
						txt->end = txt->last;
						txt->sel = 2;
						break;
					}
				}
			}

			reset_caret_timer(focus);
			refresh_window(focus);
			break;
		}
		case WM_KEYUP:
		{
			switch (wParam) {
				case VK_CONTROL:
					key_mod &= ~CTRL;
					break;
				case VK_SHIFT:
					key_mod &= ~SHIFT;
					break;
			}

			refresh_window(focus);
			break;
		}
		case WM_CHAR:
		{
			if (!focus || (key_mod & CTRL) || wParam < ' ' || wParam > '~')
				break;

			start_editing();
			insert_char(wParam);

			reset_caret_timer(focus);
			refresh_window(focus);
			break;
		}
		case WM_LBUTTONDOWN:
		{
			int x = lParam & 0xffff;
			int y = lParam >> 16;

			set_focus(window_from_coords(x, y));

			text_t *txt = focussed_text();
			if (txt) {
				set_caret_from_coords(focus, x, y);
				start_selection(txt);
			}

			lb_held = 1;
			reset_caret_timer(focus);
			refresh_window(focus);
			break;
		}
		case WM_MOUSEMOVE:
		{
			int x = lParam & 0xffff;
			int y = lParam >> 16;

			text_t *txt = focussed_text();
			if (lb_held && txt) {
				set_caret_from_coords(focus, x, y);
				update_selection(txt);
			}

			reset_caret_timer(focus);
			refresh_window(focus);
			break;
		}
		case WM_LBUTTONUP:
		{
			text_t *txt = focussed_text();

			if (txt && txt->start && txt->sel == 1 &&
				txt->start->start == txt->start->end)
					end_selection(txt);

			lb_held = 0;
			refresh_window(focus);
			break;
		}
		case WM_MOUSEWHEEL:
		{
			RECT r;
			GetWindowRect(hwnd, &r);

			int x = (lParam & 0xffff) - r.left;
			int y = (lParam >> 16) - r.top;
			set_focus(window_from_coords(x, y));

			text_t *txt = focussed_text();
			if (!txt || !txt->cur)
				break;

			short delta = wParam >> 16;
			if (delta > 0 && txt->cur->prev)
				set_row(txt, txt->cur->prev);
			else if (delta < 0 && txt->cur->next)
				set_row(txt, txt->cur->next);

			reset_caret_timer(focus);
			refresh_window(focus);
			break;
		}
		case WM_CLOSE:
			delete_brushes();
			DestroyWindow(hwnd);
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		case WM_NCHITTEST:
			return HTCLIENT;
		default:
			return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	HWND hwnd = init_gui(hInstance, WndProc);
	if (!hwnd)
		return 0;

	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	MSG Msg;
	while (GetMessage(&Msg, NULL, 0, 0) > 0) {
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}

	return Msg.wParam;
}