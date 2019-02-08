#include "header.h"
#include "asm.h"

/*
 * Eventual To-do:
 *   - Convert names of symbols into numbers before/after assembly/disassembly
 *   - ...
*/

void assemble(line_t *line) {
	error_t err = {0};
	line->equiv->str = n64_asm(line->str, &err);
	//send_error(&err);
}

void disassemble(line_t *line) {
	error_t err = {0};
	line->equiv->str = n64_disasm(line->str, &err);
	//send_error(&err);
}