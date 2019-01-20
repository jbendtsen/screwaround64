#include "header.h"

// Project Operations

void new_project(project_t *proj) {
	memset(proj, 0, sizeof(project_t));
	add_tab(proj);
	switch_tab(proj, 0);
}

void close_project(project_t *proj) {
	if (!proj) return;

	if (proj->name) free(proj->name);
	if (proj->title) free(proj->title);
	if (proj->desc) free(proj->desc);

	proj->idx = -1;

	int i;
	for (i = 0; i < proj->n_tabs; i++)
		delete_tab(proj, i);

	memset(proj, 0, sizeof(project_t));
}

// Tab Operations

void add_tab(project_t *proj) {
	int last = proj->n_tabs;
	proj->tab = realloc(proj->tab, ++proj->n_tabs * sizeof(tab_t));

	tab_t *tab = &proj->tab[last];
	create_text(&tab->asm_text);
	create_text(&tab->bin_text);

	tab->asm_text.type = ASM_WND;
	tab->bin_text.type = BIN_WND;
}

void switch_tab(project_t *proj, int idx) {
	if (idx < 0) idx = proj->n_tabs - 1;
	if (idx >= proj->n_tabs) idx = 0;

	tab_t *tab = &proj->tab[idx];
	set_texts(&tab->asm_text, &tab->bin_text);

	proj->idx = idx;
}

void delete_tab(project_t *proj, int idx) {
	tab_t *tab = &proj->tab[idx];
	close_text(&tab->asm_text);
	close_text(&tab->bin_text);

	if (tab->title) free(tab->title);
	if (tab->desc) free(tab->desc);
	memset(tab, 0, sizeof(tab_t));

	if (proj->idx == idx) {
		if (idx > 0)
			switch_tab(proj, idx - 1);
		else
			set_texts(NULL, NULL);
	}
}