#include "header.h"
#include "asm.h"

/*
 * Eventual To-do:
 *   - Convert names of symbols into numbers before/after assembly/disassembly
 *   - ...
*/

void test(line_t *line) {
	line->equiv->str = strdup(line->str);
}

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

/*
   Preprocessor
    Extracts the useful part of the string (between the first and last non-whitespace characters),
     removes it from the rest of the string,
     saves the rest of the string (as well as the offset to the useful sub-string)
*/
char *preprocess(char *input, char **edges, int *offset) {
	if (!input || !strlen(input) || !edges || !offset)
		return NULL;

	char *line = strdup(input);

	// Determine the length of the string, preceding the comment if one exists
	int len;
	for (len = 0; line[len] &&
	              line[len] != '#' &&
	              line[len] != ';' &&
	              line[len] != '\r' &&
	              line[len] != '\n';
	               len++);

	if (len < 1) {
		*edges = line;
		*offset = 0;
		return NULL;
	}

	// Find the last non-whitespace character
	int end;
	for (end = len-1; end >= 0 &&
	                 (line[end] == ' ' ||
	                  line[end] == '\t');
	                   end++);

	if (end < 0) {
		*edges = line;
		*offset = 0;
		return NULL;
	}

	// Find the first non-whitespace character
	int start;
	for (start = 0; start <= end &&
	               (line[start] == ' ' ||
	                line[start] == '\t');
	                 start++);

	if (start >= end)
		start = 0;

	// Now that the useful substring of the input string has been located, we can extract it
	len = (end - start) + 1;
	char *str = calloc(len + 1, 1);
	memcpy(str, line + start, len);

	// Remove the substring from the input
	memmove(line + start, line + end + 1, strlen(line) - len);
	memset(line + strlen(line) - len, 0, len);

	*edges = strdup(line);
	*offset = start;

	free(line);
	return str;
}

/*
   Postprocessor
    Simply inserts the output back into the original line, comments and spacing intact
*/
char *postprocess(char *output, char *edges, int offset) {
	if (!output || !strlen(output)) return NULL;
	if (!edges || !strlen(edges)) return output;

	if (offset < 0 || offset > strlen(edges))
		return output;

	char *str = calloc(strlen(output) + strlen(edges) + 1, 1);

	if (offset > 0)
		memcpy(str, edges, offset);
	if (offset < strlen(edges))
		memcpy(str + offset + strlen(output), edges + offset, strlen(edges) - offset);

	memcpy(str + offset, output, strlen(output));
	return str;
}

/*
// pre-process a, pre-process b,
// assemble a over b, disassemble b over a,
// post-process a, post-process b
char *assemble(char *input, error_t *error) {
	if (!input || !error)
		return NULL;

	memset(error, 0, sizeof(error_t));

	char *edges = NULL;
	int offset = 0;
	char *str = preprocess(input, &edges, &offset);
	if (!str) {
		if (edges) free(edges);
		return NULL;
	}

	char *rejoin = postprocess(str, edges, offset);
	if (rejoin) printf("%s", rejoin);
	else printf("n-n-n-null, n-n-n-null\n");

	char *bin = n64_asm(str, error);
	free(str);
	free(edges);

	return bin;
}
*/