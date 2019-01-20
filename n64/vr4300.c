#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vr4300.h"
#include "../asm.h"

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

#define NO_ERROR       0

#define ERR_ASM_INPUT  1
#define ERR_ASM_CHAR   2
#define ERR_ASM_MNEM   3
#define ERR_ASM_COUNT  4
#define ERR_ASM_PARAM  5

#define ERR_DIS_INPUT  6
#define ERR_DIS_CHAR   7
#define ERR_DIS_LENGTH 8
#define ERR_DIS_INS    9
#define ERR_DIS_PARAM  10

#define N_ERRORS 11

struct err_msg {
	int type;
	char problem[60];
};

struct err_msg errors[] = {
	{0,   "No error"},
	{100, "No input given"},
	{101, "Invalid character"},
	{102, "Invalid instruction"},
	{103, "Invalid arguments"},
	{104, "Invalid parameter"},
	{200, "No input given"},
	{201, "Invalid character"},
	{202, "Invalid length"},
	{203, "Invalid instruction"},
	{204, "Invalid parameter"}
};

void set_error(error_t *err, int idx) {
	if (idx < 0 || idx >= N_ERRORS)
		return;

	err->type = errors[idx].type;
	err->problem = &errors[idx].problem[0];
}

typedef struct {
	int off;
	int len;
} str_arg_t;

char reason[256];

char *reg_types[] = {"cpu reg", "cop0 reg", "cop1 reg", ""};
char *imm_types[] = {"imm", "offset", "branch", "ins_idx", "shift", "op", "code", ""};

char *_add_str(char *in, char *add) {
	if (!add || !strlen(add)) return in;
	if (!in || !strlen(in)) return strdup(add);

	int in_sz = strlen(in), add_sz = strlen(add);
	char *str = calloc(in_sz + add_sz + 1, 1);
	strcpy(str, in);
	strcpy(str + in_sz, add);

	free(in);
	return str;
}

char *stringify(u32 n) {
	char *out = calloc(9, 1);

	int i;
	for (i = 0; i < 8; i++) {
		char c = (n >> ((7 - i) * 4)) & 0xf;

		if (c <= 9)
			c += '0';
		else
			c += 'a' - 0xa;

		out[i] = c;
	}

	return out;
}

int get_reg(char *str, char prefix) {
	if (!str) return -1;

	int reg = -1;
	if (str[0] >= '0' && str[0] <= '9')
		reg = strtol(str, NULL, 10);
	else if (str[0] == prefix || str[0] == prefix ^ 0x20) // either lowercase or uppercase
		reg = strtol(str + 1, NULL, 10);

	return reg;
}

//int parse_int(char *str);

char *n64_asm(char *as, error_t *err) {
	if (!err) return NULL;

	memset(err, 0, sizeof(error_t));
	if (!as || !strlen(as)) {
		set_error(err, ERR_ASM_INPUT);
		err->col = 0;
		err->len = 0;
		return NULL;
	}

	// Convert the input string to lowercase and convert all whitespace characters to spaces
	// Additionally, check for non-ascii characters
	int i, j, rsn_len = 0;
	char *str = strdup(as);
	for (i = 0; str[i]; i++) {
		if (str[i] < ' ' || str[i] > '~') {
			set_error(err, ERR_ASM_CHAR);
			err->col = i;
			err->len = 1;

			rsn_len = snprintf(reason, 256, "unrecognised byte (%02x) at column %d\n", str[i], i+1);
			err->reason = strdup(reason);

			free(str);
			return NULL;
		}

		if (str[i] >= 'A' && str[i] <= 'Z')
			str[i] |= 0x20;
	}

	// Find the length of the instruction mnemonic from the input string
	// Equivalent to getting the instruction mnemonic itself
	int nlen;
	for (nlen = 0; nlen < 16 &&
	               str[nlen] &&
	               str[nlen] != ' ';
	                 nlen++);

	if (!memcmp(str, "nop", 3)) {
		free(str);
		return stringify(0);
	}

	// Find the correct entry in the instruction table by the mnemonic
	mips_ins entry = {0};
	for (i = 0; i < N_INS; i++) {
		if (strlen(mips_instruction_set[i].name) == nlen &&
		    !memcmp(str, mips_instruction_set[i].name, nlen)) {
			entry = mips_instruction_set[i];
			break;
		}
	}
	if (!strlen(entry.name)) {
		set_error(err, ERR_ASM_MNEM);
		err->col = 0;
		err->len = nlen;

		rsn_len = snprintf(reason, 256, "unrecognised mnemonic (\"%*s\")\n", nlen, str);
		err->reason = strdup(reason);

		free(str);
		return NULL;
	}

	str_arg_t *arg = NULL;
	int n_args = 0;

	// Get a list of the arguments by finding the offset and length of each argument contained inside the input string
	if (nlen < strlen(str)) {
		int pos = nlen + 1;
		for (i = nlen + 1; ; i++) {
			if (str[i] != '\0' && str[i] != ',' && str[i] != '(')
				continue;

			while (pos < strlen(str) && str[pos] == ' ') pos++;

			arg = realloc(arg, ++n_args * sizeof(str_arg_t));
			arg[n_args - 1].off = pos;
			arg[n_args - 1].len = i - pos;

			if (str[i] == ',' || str[i] == '(') {
				pos = i + 1;
				continue;
			}
			else break;
		}
	}

	/*
	printf("n_args: %d\n", n_args);
	if (arg && n_args) {
		for (i = 0; i < n_args; i++) {
			char *a = calloc(arg[i].len, 1);
			memcpy(a, str + arg[i].off, arg[i].len);
			printf("arg[%d].off: %d, arg[%d].len: %d\nstring of arg[%d]: \"%s\"\n",
				i, arg[i].off, i, arg[i].len, i, a);
			free(a);
		}
	}
	*/

	// Get the list of expected arguments from the entry in the instruction table
	u16 order = entry.order;
	int n_ops; // number of operands (arguments)
	for (n_ops = 0; n_ops < 4 && (order >> 12) < 4; n_ops++, order <<= 4);

	if (n_args != n_ops) {
		set_error(err, ERR_ASM_COUNT);
		err->col = arg ? arg[0].off : strlen(entry.name);
		err->len = strlen(str) - err->col;

		char *cmp_str = n_args > n_ops ? "too many" : "not enough";
		rsn_len = snprintf(reason, 256, "%s parameters (%d given, %d needed)\n", cmp_str, n_args, n_ops);
		err->reason = strdup(reason);

		free(str);
		free(arg);
		return NULL;
	}

	// Initialise the instruction blocks with constant values (eg. operation id, etc)
	u8 block[6] = {0};
	for (i = 0; i < 6; i++)
		if (entry.block[i] & 0x80) block[i] = entry.block[i] & 0x7f;

	// Encode the input arguments into the instruction blocks
	order = entry.order;
	for (i = 0; i < n_args; i++, order <<= 4) {
		int place = (order >> 12) + 1;
		if (place > 4) continue;

		u8 type = entry.block[place];
		if (type & CONST_FLAG) continue;

		//printf("place: %d, type: %#x\n", place, type);

		int off = arg[i].off;
		int len = arg[i].len;
		int arg_err = 0;
		if (type == CPU_REG) {
			// check to see if the register is given by its name
			int reg = -1;
			for (j = 0; j < N_CPU_REGS; j++) {
				if (len >= strlen(cpu_regs[j]) && !memcmp(str + off, cpu_regs[j], strlen(cpu_regs[j]))) {
					reg = j;
					break;
				}
			}

			// otherwise, check if it's presented by its index
			if (reg < 0)
				reg = get_reg(str + off, 'r');

			if (reg < 0 || reg >= N_CPU_REGS)
				arg_err = 1; // unrecognised register
			else
				block[place] = reg;
		}
		else if (type == COP0_REG) {
			int reg = -1;
			for (j = 0; j < N_COP0_REGS; j++) {
				if (len >= strlen(cop0_regs[j]) && !memcmp(str + off, cop0_regs[j], strlen(cop0_regs[j]))) {
					reg = j;
					break;
				}
			}

			if (reg < 0)
				reg = get_reg(str + off, 'r');

			if (reg < 0 || reg >= N_COP0_REGS)
				arg_err = 1;
			else
				block[place] = reg;
		}
		else if (type == COP1_REG) { // cop1 (floating-point) register
			// find the first number
			int reg = get_reg(str + off, 'f');
			if (reg < 0 || reg > N_COP1_REGS)
				arg_err = 1;
			else
				block[place] = reg;
		}

		else { // immediate number
			// create a mask in which the range of bits not set represent the range of allowed values
			int mask = -1; // every bit set
			for (j = place; j < 6 && entry.block[j] == type; j++)
				mask <<= j == 5 ? 6 : 5;

			int n_blocks = j - place;

			int neg = 0;
			if (str[off] == '-') {
				neg = 1;
				off++;
			}

			int imm;
			if (str[off] == '$' && off < strlen(str) - 1)
				imm = strtol(str + off + 1, NULL, 16);
			else
				imm = strtol(str + off, NULL, 0);

			if (neg)
				imm = -imm;

			if (imm < 0) // make sure negative numbers are handled properly
				imm &= 0xffff;

			// if imm has bits that don't fit into the defined range, imm is too big
			if (imm & mask)
				arg_err = 2;
			else {
				for (j = place + n_blocks - 1; j >= place; j--) {
					if (j == 5) {
						block[j] = imm & 0x3f;
						imm >>= 6;
					}
					else {
						block[j] = imm & 0x1f;
						imm >>= 5;
					}
				}
			}
		}

		if (arg_err) {
			set_error(err, ERR_ASM_PARAM);
			err->col = off;
			err->len = len;

			if (arg_err == 1)
				rsn_len = snprintf(reason, 256, "parameter %d - unrecognised register (\"%*s\")\n", i+1, len, str + off);
			else
				rsn_len = snprintf(reason, 256, "parameter %d - %*s is too large\n", i+1, len, str + off);

			err->reason = strdup(reason);

			free(str);
			free(arg);
			return NULL;
		}

		//printf("place: %d, block[place]: %#x\n", place, block[place]);
	}

	free(str);
	free(arg);

	// Arrange the instruction blocks into a 32-bit unsigned integer
	u32 ins = 0;
	ins |= (block[0] & 0x3f) << 26;
	ins |= (block[1] & 0x1f) << 21;
	ins |= (block[2] & 0x1f) << 16;
	ins |= (block[3] & 0x1f) << 11;
	ins |= (block[4] & 0x1f) << 6;
	ins |= (block[5] & 0x3f);

	// Encode the integer into a hexadecimal string
	return stringify(ins);
}

char *n64_disasm(char *mach, error_t *err) {
	if (!err) return NULL;

	memset(err, 0, sizeof(error_t));
	if (!mach || !strlen(mach)) {
		set_error(err, ERR_DIS_INPUT);
		return NULL;
	}

	int rsn_len = 0;
	u32 ins = 0;
	int i, j, x, count = 0;
	for (i = 0; mach[i]; i++) {
		if (mach[i] == '-' || mach[i] == ' ' || mach[i] == '\t' ||
			mach[i] == '\n' || mach[i] == '\r')
				continue;

		if (mach[i] >= '0' && mach[i] <= '9')
			x = mach[i] - '0';

		else if (mach[i] >= 'A' && mach[i] <= 'F')
			x = mach[i] - 'A' + 0xa;

		else if (mach[i] >= 'a' && mach[i] <= 'f')
			x = mach[i] - 'a' + 0xa;

		else {
			set_error(err, ERR_DIS_CHAR);
			err->col = i;
			err->len = 1;

			if (mach[i] >= ' ' && mach[i] <= '~')
				rsn_len = snprintf(reason, 256, "unrecognised character (%c)\n", mach[i]);
			else
				rsn_len = snprintf(reason, 256, "unrecognised byte (%02x)\n", mach[i]);

			err->reason = calloc(rsn_len + 1, 1);
			memcpy(err->reason, reason, rsn_len);

			return NULL;
		}

		if (count >= 8)
			break;

		ins <<= 4;
		ins |= x & 0xf;
		count++;
	}

	if (count != 8) {
		set_error(err, ERR_DIS_LENGTH);
		err->col = i;
		err->len = strlen(mach) - i;

		rsn_len = snprintf(reason, 256, "%s characters (given: %d, needed: 8)",
			count < 8 ? "not enough" : "too many", count);
		err->reason = strdup(reason);

		return NULL;
	}

	if (ins == 0)
		return strdup("nop");

	u8 block[6];
	block[0] = (ins >> 26) & 0x3f;
	block[1] = (ins >> 21) & 0x1f;
	block[2] = (ins >> 16) & 0x1f;
	block[3] = (ins >> 11) & 0x1f;
	block[4] = (ins >>  6) & 0x1f;
	block[5] =  ins        & 0x3f;

	int idx = -1, bl;
	mips_ins entry;
	for (i = 0; i < N_INS; i++) {
		entry = mips_instruction_set[i];
		int n_const = 0, n_matches = 0;
		for (j = 0; j < 6; j++) {
			bl = entry.block[j];
			if ((bl & 0x80) == 0)
				continue;

			n_const++;
			if (block[j] == (bl & 0x7f))
				n_matches++;
		}
		if (n_const > 0 && n_matches == n_const) {
			idx = i;
			break;
		}
	}
	if (idx < 0) {
		set_error(err, ERR_DIS_INS);
		err->col = 0;
		err->len = strlen(mach);
		return NULL;
	}

	char *str = _add_str(strdup(entry.name), " ");
	char num[16] = {0};

	int offset = 0;
	u16 order = entry.order;
	for (i = 0; i < 4; i++, order <<= 4) {
		int place = order >> 12;
		if (place > 3)
			break;

		bl = block[place + 1];
		u8 type = entry.block[place + 1];

		if (offset)
			str = _add_str(str, "(");
		else if (i > 0)
			str = _add_str(str, ",");

		//printf("type: %#x, bl: %#x\n", type, bl);

		if (type == CPU_REG && bl < 0x20)
			str = _add_str(str, cpu_regs[bl]);
		else if (type == COP0_REG && bl < 0x18)
			str = _add_str(str, cop0_regs[bl]);
		else if (type == COP1_REG && bl < 0x20)
			str = _add_str(str, cop1_regs[bl]);

		else if (type < 0x40) {
			int n = 0;
			for (j = place + 1; j < 6 && entry.block[j] == type; j++) {
				if (j == 5)
					n = (n << 6) | (block[j] & 0x3f);
				else
					n = (n << 5) | (block[j] & 0x1f);
			}

			memset(num, 0, 16);
			sprintf(num, "%#x", n);
			str = _add_str(str, num);
		}

		else {
			set_error(err, ERR_DIS_PARAM);
			err->col = 0;
			err->len = strlen(mach);

			if ((type >= IMM && type <= IMM_CODE) || (type >= CPU_REG && type < COP1_REG))
				snprintf(reason, 256, "parameter %d - invalid %s: value: %d",
					i, (type & 0x40) ? reg_types[type & 3] : imm_types[type & 7], bl);
			else
				snprintf(reason, 256, "parameter %d, block %d - type: %d, value: %d", i, place + 1, type, bl);

			err->reason = strdup(reason);

			free(str);
			return NULL;
		}

		if (offset)
			str = _add_str(str, ")");

		if (type == IMM_OFFSET)
			offset = 1;
		else
			offset = 0;
	}

	return str;
}