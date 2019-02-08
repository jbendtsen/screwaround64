#include "header.h"

void init_text(text_t *txt) {
	txt->first = calloc(1, sizeof(line_t));
	txt->top = txt->cur = txt->last = txt->first;
}

void create_texts(tab_t *tab) {
	init_text(&tab->asm_text);
	init_text(&tab->bin_text);
	init_text(&tab->err_list);
	init_text(&tab->err_info);

	text_t *a = &tab->asm_text;
	text_t *b = &tab->bin_text;

	a->first->equiv = b->first;
	b->first->equiv = a->first;
}

void close_text(text_t *text) {
	line_t *line = text->first;
	while (line) {
		if (line->str) free(line->str);
		line_t *temp = line;
		line = line->next;
		free(temp);
	}

	memset(text, 0, sizeof(text_t));
}

// Add a new line, where 'equivalent' is the line we just created (starts off as NULL)
void add_line(text_t *text, line_t *equivalent, int above) {
	// Create a new line
	line_t *line = calloc(1, sizeof(line_t));
	// Get the line beneath the current line
	line_t *current = text->cur;
	line_t *beneath = current->next;

	// If there is a line beneath the current line, set its previous line to be the new line
	if (beneath) {
		line->next = beneath;
		beneath->prev = line;
	}

	// Set the new line to be the next line to the current line
	line->prev = current;
	current->next = line; 

	// If the current line was the last line, ensure the last line points to our new line
	if (text->last == current)
		text->last = line;

	// If the new line was meant to be above the current line, swap the properties of the two lines
	if (above && current->str) {
		line->str = current->str;
		line->start = current->start;
		line->end = current->end;
		line->col = current->col;

		current->str = NULL;
		current->start = current->end = current->col = 0;
	}

	// If the equivalent line has already been created, link that and this new line together
	if (equivalent) {
		line->equiv = equivalent;
		equivalent->equiv = line;
	}
	// Otherwise, add a new line using this new line as the equivalent line
	else {
		text_t *other = opposed_text(text);
		if (other)
			add_line(other, line, above);
	}
}

int line_count(line_t *top, line_t *bottom) {
	int i = 1;
	while (top && top != bottom) {
		top = top->next;
		i++;
	}

	return i;
}

int find_line(line_t *start, line_t *end) {
	if (!start || !end || start == end)
		return 0;

	int i = 0;
	line_t *line = start;
	while (line && line != end) {
		line = line->next;
		i++;
	}

	if (line == end)
		return i;

	i = 0;
	line = start;
	while (line && line != end) {
		line = line->prev;
		i++;
	}

	if (line == end)
		return -i;

	return 0;
}

line_t *walk_lines(line_t *line, int count) {
	if (!line || count == 0)
		return line;

	int back = 0;
	if (count < 0) {
		count = -count;
		back = 1;
	}

	int i;
	for (i = 0; i < count; i++) {
		line_t *next = back ? line->prev : line->next;
		if (!next)
			break;
		else
			line = next;
	}

	return line;
}

line_t *resolve_line(line_t *line, line_t *test) {
	if (line != test)
		return line;

	if (!line || (!line->prev && !line->next))
		return NULL;

	if (!line->prev)
		line = line->next;
	else
		line = line->prev;

	return line;
}

void clear_line(text_t *text, line_t *line, int completely) {
	if (!line)
		return;

	if (line->str) {
		free(line->str);
		line->str = NULL;
		line->col = 0;
	}

	if (!completely || (!line->prev && !line->next))
		return;

	if (line->prev) line->prev->next = line->next;
	if (line->next) line->next->prev = line->prev;

	text->first = resolve_line(text->first, line);
	text->last = resolve_line(text->last, line);
	text->start = resolve_line(text->start, line);
	text->end = resolve_line(text->end, line);
	text->top = resolve_line(text->top, line);
	text->cur = resolve_line(text->cur, line);

	if (line->equiv) {
		line->equiv->equiv = NULL;
		clear_line(opposed_text(text), line->equiv, 1);
	}

	memset(line, 0, sizeof(line_t));
	free(line);
}

void start_selection(text_t *text) {
	if (!text->cur)
		return;

	text->start = text->cur;
	text->end = text->cur;
	text->cur->start = text->cur->col;
	text->cur->end = text->cur->col;
	text->sel = 1;
}

void end_selection(text_t *text) {
	text->start = NULL;
	text->end = NULL;
	text->sel = 0;
}

void update_selection(text_t *text) {
	if (!text->sel || !text->cur || !text->start)
		return;

	text->end = text->cur;
	if (text->cur == text->start) {
		text->cur->end = text->cur->col;
		text->sel = 1;
		return;
	}

	line_t *above = text->start->prev;
	line_t *below = text->start->next;

	while (above || below) {
		if (text->cur == above) {
			text->sel = 3;
			return;
		}
		if (text->cur == below) {
			text->sel = 2;
			return;
		}

		if (above) above = above->prev;
		if (below) below = below->next;
	}

	text->sel = 0;
}

void delete_selection(text_t *text) {
	if (!text->sel || !text->start || !text->end)
		return;

	if (text->sel == 1) {
		if (!text->start->str) return;

		int a = text->start->start;
		int b = text->start->end;
		int len = strlen(text->start->str);

		if (a > len) a = len;
		if (b > len) b = len;
		if (a == b)
			return;

		if (a > b) {
			int temp = a;
			a = b;
			b = temp;
		}

		memmove(text->start->str + a, text->start->str + b, len - b);
		text->start->str[len - (b - a)] = 0;

		char *new_str = strdup(text->start->str);
		free(text->start->str);
		text->start->str = new_str;

		set_row(text, text->start);
		set_column(text, a);
	}
	else {
		line_t *a = text->start;
		line_t *b = text->end;
		if (!a || !b) return;

		if (text->sel == 3) {
			line_t *temp = a;
			a = b;
			b = temp;
		}

		line_t *walk = b;
		do {
			line_t *temp = walk->prev;
			clear_line(text, walk, 1);
			walk = temp;
		} while (walk && walk != a);

		clear_line(text, a, 0);
		set_row(text, a);
	}

	end_selection(text);
}

#define ARROW_LEFT  0
#define ARROW_UP    1
#define ARROW_RIGHT 2
#define ARROW_DOWN  3

void move_cursor(int dir, int shift) {
	text_t *txt = focussed_text();
	if (!txt->cur) {
		//printf("No focus, no ambition, no nothin\n");
		return;
	}

	if (!txt->cur->str && (dir == ARROW_LEFT || dir == ARROW_RIGHT))
		return;

	if (shift && !txt->sel)
		start_selection(txt);
	if (!shift && txt->sel)
		end_selection(txt);

	int len = 0;
	if (txt->cur->str)
		len = strlen(txt->cur->str);

	if (dir == ARROW_LEFT && txt->cur->col > 0)
		set_column(txt, txt->cur->col - 1);

	else if (dir == ARROW_RIGHT)
		set_column(txt, txt->cur->col + 1);

	else if (dir == ARROW_UP || dir == ARROW_DOWN) {
		int col = txt->column;
		if (dir == ARROW_UP) {
			line_t *above = txt->cur->prev ? txt->cur->prev : txt->first;
			set_row(txt, above);
		}
		else {
			line_t *below = txt->cur->next ? txt->cur->next : txt->last;
			set_row(txt, below);
		}

		set_column(txt, col);
	}
}

void delete_text(int back, int shift) {
	text_t *txt = focussed_text();
	if (!txt || !txt->cur)
		return;

	if (txt->sel) {
		delete_selection(txt);
		return;
	}

	char *str = txt->cur->str;
	if (!str) {
		if (shift) clear_line(txt, txt->cur, 1); // clear line completely
		return;
	}

	if (shift || strlen(str) <= 1) {
		clear_line(txt, txt->cur, 0); // clear line, but leave it empty
		return;
	}

	int pos = txt->cur->col;

	if (back || pos == strlen(str)) pos--;
	if (pos < 0) return;

	set_column(txt, pos);

	char *latter = txt->cur->str + txt->cur->col;
	memmove(latter, latter + 1, strlen(latter) - 1);
	latter[strlen(latter) - 1] = 0;

	txt->cur->str = realloc(txt->cur->str, strlen(txt->cur->str) + 1);

	if (!txt->sel)
		txt->cur->end = txt->cur->col;
}

void insert_char(char ch) {
	text_t *text = focussed_text();
	if (!text || ch < ' ' || ch > '~')
		return;

	if (text->sel)
		delete_selection(text);

	if (!text->cur->str) {
		text->cur->str = calloc(2, 1);
		text->cur->str[0] = ch;
		set_column(text, 1);
		text->cur->end = text->cur->col;
		return;
	}

	line_t *line = text->cur;

	int pos = line->col;
	int len = strlen(line->str);

	if (pos < 0 || pos > len)
		return;

	line->str = realloc(line->str, len + 2);

	if (pos < len)
		memmove(line->str + pos + 1, line->str + pos, len - pos);

	line->str[pos] = ch;
	line->str[len + 1] = 0;

	set_column(text, line->col + 1);
	line->end = line->col;
}

char *addstr(char *str, char *add, int *pos) {
	if (!add) return str;

	int str_pos = pos ? *pos : 0;
	int add_len = strlen(add);

	str = realloc(str, str_pos + add_len + 1);
	strcpy(str + str_pos, add);
	str[str_pos + add_len] = 0;

	if (pos) *pos = str_pos + add_len;
	return str;
}

void copy_text(int cut) {
	text_t *txt = focussed_text();
	if (!txt || !txt->sel)
		return;

	char *clip = NULL;
	if (txt->sel == 1) {
		if (!txt->start || !txt->start->str)
			return;

		int a = txt->start->start;
		int b = txt->start->end;
		int len = strlen(txt->start->str);

		if (a > len) a = len;
		if (b > len) b = len;
		if (a == b)
			return;

		if (a > b) {
			int temp = a;
			a = b;
			b = temp;
		}

		clip = calloc(b - a + 1, 1);
		memcpy(clip, txt->start->str + a, b - a);
	}
	else {
		line_t *a = txt->start;
		line_t *b = txt->end;
		if (!a || !b) return;

		if (txt->sel == 3) {
			line_t *temp = a;
			a = b;
			b = temp;
		}

		int pos = 0;
		line_t *walk = a;
		while (walk) {
			clip = addstr(clip, walk->str, &pos);
			if (walk == b) break;
			clip = addstr(clip, "\r\n", &pos);
			walk = walk->next;
		}
	}

	if (clip) {
		int len = strlen(clip) + 1;
		HGLOBAL mem = GlobalAlloc(GMEM_MOVEABLE, len);

		memcpy(GlobalLock(mem), clip, len);
		GlobalUnlock(mem);

		OpenClipboard(NULL);
		EmptyClipboard();
		SetClipboardData(CF_TEXT, mem);
		CloseClipboard();

		free(clip);
	}

	if (cut) delete_selection(txt);
}

void paste_text() {
	text_t *txt = focussed_text();
	if (!txt) return;

	if (!OpenClipboard(NULL))
		return;

	HANDLE mem = GetClipboardData(CF_TEXT);
	if (!mem) return;

	char *clip = GlobalLock(mem);
	if (!clip || !strlen(clip)) {
		CloseClipboard();
		return;
	}
	int clip_len = strlen(clip);

	if (txt->sel)
		delete_selection(txt);

	int i, newlines = 0;
	for (i = 0; clip[i]; i++)
		if (clip[i] == '\n') newlines++;

	if (!newlines) {
		if (txt->cur->str) {
			int len = strlen(txt->cur->str);
			int col = txt->cur->col;

			txt->cur->str = realloc(txt->cur->str, len + clip_len + 1);

			if (col < len)
				memmove(txt->cur->str + col + clip_len, txt->cur->str + col, len - col);

			memcpy(txt->cur->str + col, clip, clip_len);

			txt->cur->col += clip_len;
		}
		else {
			txt->cur->str = strdup(clip);
			txt->cur->col = clip_len;
		}
	}
	else {
		int start = 0;
		while (start < clip_len) {
			if (txt->cur->str) {
				// create_new_line()???
				add_line(txt, NULL, 0);
				set_row(txt, txt->cur->next);
			}

			for (i = start;
			     clip[i] && clip[i] != '\r' && clip[i] != '\n';
			     i++);

			int len = i - start;
			txt->cur->str = calloc(len + 1, 1);
			memcpy(txt->cur->str, clip + start, len);

			for (start = i; clip[start] == '\r' || clip[start] == '\n'; start++) {
				if (!clip[start] || start >= clip_len) {
					start = clip_len;
					break;
				}
			}
		}

		txt->cur->col = strlen(txt->cur->str);
	}

	GlobalUnlock(mem);
	CloseClipboard();
}

void set_column(text_t *text, int col) {
	if (!text || !text->cur)
		return;

	if (!text->cur->str) {
		text->cur->col = 0;
		text->column = 0;
		return;
	}

	int len = strlen(text->cur->str);
	if (col < 0 || col > len)
		col = len;

	text->cur->col = col;
	text->column = col;

	update_selection(text);
}

void set_row(text_t *text, line_t *row) {
	int idx = window_from_text(text);
	if (idx < 0 || !text || !row)
		return;

	process_line();
	text->cur = row;

	int seek = find_line(text->top, text->cur);
	if (seek < 0)
		text->top = text->cur;
	else {
		int n_lines = calc_visible_lines(idx, text->top);
		if (seek >= n_lines)
			text->top = walk_lines(text->top, seek - n_lines + 1);
	}

	text_t *other = opposed_text(text);
	if (other) {
		other->cur = text->cur->equiv;
		other->top = text->top->equiv;
	}
}