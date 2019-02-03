#include "header.h"

#define CTRL  1
#define SHIFT 2

project_t ctx;

int focus = 0;
int hover = 0;

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

void set_focus(int wnd) {
	if (focus != wnd)
		stop_editing();

	int old_focus = focus;
	focus = wnd;

	reset_caret_timer(focus);
	refresh_window(old_focus);
}

text_t *asm_text_cur = NULL;
text_t *bin_text_cur = NULL;

void set_texts(text_t *asm_txt, text_t *bin_txt) {
	asm_text_cur = asm_txt;
	bin_text_cur = bin_txt;
}

text_t *text_of(int wnd) {
	if (wnd == ASM_WND)
		return asm_text_cur;
	if (wnd == BIN_WND)
		return bin_text_cur;

	return NULL;
}

text_t *opposed_text(text_t *text) {
	if (text == asm_text_cur)
		return bin_text_cur;
	if (text == bin_text_cur)
		return asm_text_cur;

	return NULL;
}

void process_line() {
	if (focus == ASM_WND)
		assemble(asm_text_cur->cur);
	else if (focus == BIN_WND)
		disassemble(bin_text_cur->cur);

	stop_editing();
}

int key_mod = 0;
int lb_held = 0;

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	set_window(MAIN_WND, hwnd);

	switch(uMsg) {
		case WM_CREATE:
		{
			init_debug();

			new_project(&ctx);

			resize_mainwnd(WIDTH, HEIGHT);

			focus = ASM_WND;
			break;
		}
		case WM_SIZE:
		{
			RECT rect;

			GetClientRect(hwnd, &rect);
			int width = rect.right;
			int height = rect.bottom;

			resize_mainwnd(width, height);
			break;
		}
		case WM_CTLCOLORSTATIC:
		{
			int wnd = get_window_index((HWND)lParam);
			if (wnd != ASM_HEAD && wnd != BIN_HEAD)
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
					set_column(text_of(focus), 0);
					break;
				case VK_END:
					set_column(text_of(focus), -1);
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
					text_t *txt = text_of(focus);
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
						text_t *txt = text_of(focus);

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

			lb_held = 1;
			if (focus) {
				set_caret_from_coords(focus, x, y);
				start_selection(text_of(focus));
			}

			reset_caret_timer(focus);
			refresh_window(focus);
			break;
		}
		case WM_MOUSEMOVE:
		{
			int x = lParam & 0xffff;
			int y = lParam >> 16;

			if (lb_held && focus) {
				set_caret_from_coords(focus, x, y);
				update_selection(text_of(focus));
			}

			reset_caret_timer(focus);
			refresh_window(focus);
			break;
		}
		case WM_LBUTTONUP:
		{
			text_t *txt = text_of(focus);

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

			text_t *txt = text_of(focus);
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