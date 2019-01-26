#include "header.h"

#define CTRL  1
#define SHIFT 2

#define ID_OFF 32

int n_ids = 0;

project_t ctx;

int focus = 0;
int hover = 0;

HWND main_wnd = NULL;

HWND spawn_window(int ex_style, const char *class, const char *name, int style, int x, int y, int w, int h) {
	int id = ID_OFF + n_ids++;

	return CreateWindowEx(ex_style, class, name, style, x, y, w, h,
	                      main_wnd, (HMENU)id, GetModuleHandle(NULL), NULL);
}

int editing = 0;

void start_editing() {
	SetWindowText(main_wnd, "* Screwaround64");
	editing = 1;
}
void stop_editing() {
	SetWindowText(main_wnd, "Screwaround64");
	//process_line();
	editing = 0;
}

int get_focus() {
	return focus;
}

void set_focus(int wnd) {
	if (focus != wnd)
		stop_editing();

	focus = wnd;
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

void update_display(int width, int height) {
	int w = (width - (X_OFF * 2) - MID_GAP) / 2;
	int h = height - Y_OFF - BOTTOM;

	if (w < 0) w = 0;
	if (h < 0) h = 0;

	int w_bin = w < MAX_BIN_WIDTH ? w : MAX_BIN_WIDTH;
	int w_asm = w + (w - w_bin);
	int x_bin = X_OFF + w_asm + MID_GAP;

	resize_display(X_OFF, w_asm, x_bin, w_bin, Y_OFF, h);
}

// #define DEBUG
#ifdef DEBUG

void send_debug_msg(UINT uMsg, WPARAM wParam, LPARAM lParam) {
	HWND client = FindWindow("Client", NULL);
	SendMessage(client, uMsg, wParam, lParam);
}

void debug_string(char *str) {
	if (str && strlen(str))
		send_debug_msg(0x4001, (WPARAM)str, strlen(str));
}

struct var {
	char *label;
	int type;
	void *value;
};

struct dbg {
	char *name;
	int count;
	struct var *var;
	int n_vars;
};

#define BUF_SIZE 400
char buf[BUF_SIZE];
int dbg_ok = 0;

void debug_out(struct dbg *dbg) {
	if (!dbg)
		return;

	/*
	if (uMsg != WM_PAINT) {
		memset(buf, 0, BUF_SIZE);
		sprintf(buf, "%s %d - uMsg: %#x, wParam: %#x, lParam: %#x", dbg->name, dbg->count, uMsg, wParam, lParam);
		send_debug_msg(0x4001, (WPARAM)&buf[0], strlen(buf));
	}
	*/

	memset(buf, 0, BUF_SIZE);
	int n = snprintf(buf, BUF_SIZE, "%s %d - ", dbg->name, dbg->count);
	int pr = BUF_SIZE - n;

	const char *dec_str = "%s%s: %d";
	const char *hex_str = "%s%s: %#x";

	int i;
	for (i = 0; i < dbg->n_vars; i++) {
		const char *msg = dbg->var[i].type ? hex_str : dec_str;
		n = snprintf(buf + strlen(buf), pr, msg, i == 0 ? "" : ", ", dbg->var[i].label, dbg->var[i].value);
		pr -= n;
	}

	send_debug_msg(0x4001, (WPARAM)&buf[0], strlen(buf));
	dbg->count++;
}

struct var msg_vars[] = {
	{"hwnd", 1, NULL},
	{"uMsg", 1, NULL},
	{"wParam", 1, NULL},
	{"lParam", 1, NULL}
};

struct dbg msg_dbg = {
	"mousewheel", 0, &msg_vars[0], 4
};

void debug_winmsg(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	msg_vars[0].value = (void*)hwnd;
	msg_vars[1].value = (void*)uMsg;
	msg_vars[2].value = (void*)wParam;
	msg_vars[3].value = (void*)lParam;
	debug_out(&msg_dbg);
}

#endif

int key_mod = 0;
int lb_held = 0;

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	main_wnd = hwnd;

	switch(uMsg) {
		case WM_CREATE:
		{
		#ifdef DEBUG
			send_debug_msg(0x4000, 0, GetCurrentProcessId());
			dbg_ok = 1;
		#endif

			new_project(&ctx);

			focus = ASM_WND;
			update_display(WIDTH, HEIGHT);
			break;
		}
		case WM_SIZE:
		{
			RECT rect;

			GetClientRect(hwnd, &rect);
			int width = rect.right;
			int height = rect.bottom;

			update_display(width, height);
			break;
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

			refresh();
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

			refresh();
			break;
		}
		case WM_CHAR:
		{
			if (!focus || (key_mod & CTRL) || wParam < ' ' || wParam > '~')
				break;

			start_editing();
			insert_char(wParam);
			refresh();
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

			refresh();
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

			refresh();
			break;
		}
		case WM_LBUTTONUP:
		{
			text_t *txt = text_of(focus);

			if (txt && txt->start && txt->sel == 1 &&
				txt->start->start == txt->start->end)
					end_selection(txt);

			lb_held = 0;
			refresh();
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

			refresh();
			break;
		}
		case WM_CLOSE:
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

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, int nCmdShow)
{
	WNDCLASSEX wc;
	MSG Msg;
	char *g_szClassName = "SC64";

	wc.cbSize		 = sizeof(WNDCLASSEX);
	wc.style		 = CS_GLOBALCLASS | CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc	 = WndProc;
	wc.cbClsExtra	 = 0;
	wc.cbWndExtra	 = 0;
	wc.hInstance	 = hInstance;
	wc.hIcon		 = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor		 = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = CreateSolidBrush(BACKGROUND);
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = g_szClassName;
	wc.hIconSm		 = LoadIcon(NULL, IDI_APPLICATION);

	if (!RegisterClassEx(&wc)) {
		MessageBox(NULL, "Window Registration Failed!", "Error!",
			MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	main_wnd = CreateWindowEx(
		WS_EX_CLIENTEDGE,
		g_szClassName,
		NAME,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, WIDTH, HEIGHT,
		NULL, NULL, hInstance, NULL);

	if (!main_wnd) {
		MessageBox(NULL, "Window Creation Failed!", "Error!",
			MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	ShowWindow(main_wnd, nCmdShow);
	UpdateWindow(main_wnd);

	while(GetMessage(&Msg, NULL, 0, 0) > 0) {
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}

	return Msg.wParam;
}
