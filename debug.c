// #define DEBUG
#ifdef DEBUG

#include "header.h"

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
	if (!dbg_ok || !dbg)
		return;

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

struct var xy_vars[] = {
	{"X", 0, NULL},
	{"Y", 0, NULL},
};

struct dbg xy_dbg = {
	"Timer Creation", 0, &xy_vars[0], 2
};

void debug_winmsg(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	msg_vars[0].value = (void*)hwnd;
	msg_vars[1].value = (void*)uMsg;
	msg_vars[2].value = (void*)wParam;
	msg_vars[3].value = (void*)lParam;
	debug_out(&msg_dbg);
}

void debug_xy(int x, int y) {
	xy_vars[0].value = (void*)x;
	xy_vars[1].value = (void*)y;
	debug_out(&xy_dbg);
}

#endif

void init_debug(void) {
#ifdef DEBUG
	send_debug_msg(0x4000, 0, GetCurrentProcessId());
	dbg_ok = 1;
#endif
}