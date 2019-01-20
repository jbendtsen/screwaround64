#ifndef SC64_ASM_H
#define SC64_ASM_H

typedef struct {
	int col;
	int len;
	int type;
	char *problem;
	char *reason;
} error_t;

char *n64_asm(char *as, error_t *err);
char *n64_disasm(char *mach, error_t *err);

#endif