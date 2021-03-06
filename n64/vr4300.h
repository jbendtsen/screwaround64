#ifndef VR4300_H
#define VR4300_H

typedef struct {
	char name[16];
	unsigned char block[6];
	unsigned short order;
} mips_ins;

#define CONST_FLAG 0x80

#define CPU_REG 0x40
#define COP0_REG 0x41
#define COP1_REG 0x42

#define IMM          0
#define IMM_OFFSET   1
#define IMM_BRANCH   2
#define IMM_INS_IDX  3
#define IMM_SHIFT    4
#define IMM_OP_TYPE  5
#define IMM_CODE     6

#define N_CPU_REGS  32
#define N_COP0_REGS 24
#define N_COP1_REGS 32

char *cpu_regs[] = {
	"r0", "at", "v0", "v1",
	"a0", "a1", "a2", "a3",
	"t0", "t1", "t2", "t3",
	"t4", "t5", "t6", "t7",
	"s0", "s1", "s2", "s3",
	"s4", "s5", "s6", "s7",
	"t8", "t9", "k0", "k1",
	"gp", "sp", "s8", "ra"
};

char *cop0_regs[] = {
	"Index", "EntryLo0", "EntryLo1", "Context",
	"PageMask", "Wired", "BadVAddr", "Count",
	"EntryHi", "Compare", "Status", "Cause",
	"ExceptPC", "PRevID", "Config", "LLAddr",
	"WatchLo", "WatchHi", "XContext", "PErr",
	"CacheErr", "TagLo", "TagHi", "ErrorEPC",
	"", "", "", "",
	"", "", "", ""
};

char *cop1_regs[] = {
	"f0", "f1", "f2", "f3",
	"f4", "f5", "f6", "f7",
	"f8", "f9", "f10", "f11",
	"f12", "f13", "f14", "f15",
	"f16", "f17", "f18", "f19",
	"f20", "f21", "f22", "f23",
	"f24", "f25", "f26", "f27",
	"f28", "f29", "f30", "f31"
};

#define INS_SET_0   0
#define INS_SET_1  50
#define INS_SET_2  64
#define INS_SET_3  78
#define INS_SET_4  85
#define INS_SET_5 169

#define N_INS 203

mips_ins mips_instruction_set[] = {
	{"sll",     {0x80, 0x80, 0x40, 0x40, 0x04, 0x80}, 0x213f},
	{"srl",     {0x80, 0x80, 0x40, 0x40, 0x04, 0x82}, 0x213f},
	{"sra",     {0x80, 0x80, 0x40, 0x40, 0x04, 0x83}, 0x213f},
	{"sllv",    {0x80, 0x40, 0x40, 0x40, 0x80, 0x84}, 0x210f},
	{"srlv",    {0x80, 0x40, 0x40, 0x40, 0x80, 0x86}, 0x210f},
	{"srav",    {0x80, 0x40, 0x40, 0x40, 0x80, 0x87}, 0x210f},
	{"jr",      {0x80, 0x40, 0x80, 0x80, 0x80, 0x88}, 0x0fff},
	{"jalr",    {0x80, 0x40, 0x80, 0x40, 0x80, 0x89}, 0x02ff},
	{"sync",    {0x80, 0x80, 0x80, 0x80, 0x80, 0x8f}, 0xffff},
	{"mfhi",    {0x80, 0x80, 0x80, 0x40, 0x80, 0x90}, 0x2fff},
	{"mthi",    {0x80, 0x40, 0x80, 0x80, 0x80, 0x91}, 0x0fff},
	{"mflo",    {0x80, 0x80, 0x80, 0x40, 0x80, 0x92}, 0x2fff},
	{"mtlo",    {0x80, 0x40, 0x80, 0x80, 0x80, 0x93}, 0x0fff},
	{"dsllv",   {0x80, 0x40, 0x40, 0x40, 0x80, 0x94}, 0x210f},
	{"dsrlv",   {0x80, 0x80, 0x40, 0x40, 0x40, 0x96}, 0x213f},
	{"dsrav",   {0x80, 0x80, 0x40, 0x40, 0x40, 0x97}, 0x213f},
	{"mult",    {0x80, 0x40, 0x40, 0x80, 0x80, 0x98}, 0x01ff},
	{"multu",   {0x80, 0x40, 0x40, 0x80, 0x80, 0x99}, 0x01ff},
	{"div",     {0x80, 0x40, 0x40, 0x80, 0x80, 0x9a}, 0x01ff},
	{"divu",    {0x80, 0x40, 0x40, 0x80, 0x80, 0x9b}, 0x01ff},
	{"dmult",   {0x80, 0x40, 0x40, 0x80, 0x80, 0x9c}, 0x01ff},
	{"dmultu",  {0x80, 0x40, 0x40, 0x80, 0x80, 0x9d}, 0x01ff},
	{"ddiv",    {0x80, 0x40, 0x40, 0x80, 0x80, 0x9e}, 0x01ff},
	{"ddivu",   {0x80, 0x40, 0x40, 0x80, 0x80, 0x9f}, 0x01ff},
	{"add",     {0x80, 0x40, 0x40, 0x40, 0x80, 0xa0}, 0x201f},
	{"addu",    {0x80, 0x40, 0x40, 0x40, 0x80, 0xa1}, 0x201f},
	{"sub",     {0x80, 0x40, 0x40, 0x40, 0x80, 0xa2}, 0x201f},
	{"subu",    {0x80, 0x40, 0x40, 0x40, 0x80, 0xa3}, 0x201f},
	{"and",     {0x80, 0x40, 0x40, 0x40, 0x80, 0xa4}, 0x201f},
	{"or",      {0x80, 0x40, 0x40, 0x40, 0x80, 0xa5}, 0x201f},
	{"xor",     {0x80, 0x40, 0x40, 0x40, 0x80, 0xa6}, 0x201f},
	{"nor",     {0x80, 0x40, 0x40, 0x40, 0x80, 0xa7}, 0x201f},
	{"slt",     {0x80, 0x40, 0x40, 0x40, 0x80, 0xaa}, 0x201f},
	{"sltu",    {0x80, 0x40, 0x40, 0x40, 0x80, 0xab}, 0x201f},
	{"dadd",    {0x80, 0x40, 0x40, 0x40, 0x80, 0xac}, 0x201f},
	{"daddu",   {0x80, 0x40, 0x40, 0x40, 0x80, 0xad}, 0x201f},
	{"dsub",    {0x80, 0x40, 0x40, 0x40, 0x80, 0xae}, 0x201f},
	{"dsubu",   {0x80, 0x40, 0x40, 0x40, 0x80, 0xaf}, 0x201f},
	{"tge",     {0x80, 0x40, 0x40, 0x06, 0x06, 0xb0}, 0x012f},
	{"tgeu",    {0x80, 0x40, 0x40, 0x06, 0x06, 0xb1}, 0x012f},
	{"tlt",     {0x80, 0x40, 0x40, 0x06, 0x06, 0xb2}, 0x012f},
	{"tltu",    {0x80, 0x40, 0x40, 0x06, 0x06, 0xb3}, 0x012f},
	{"teq",     {0x80, 0x40, 0x40, 0x06, 0x06, 0xb4}, 0x012f},
	{"tne",     {0x80, 0x40, 0x40, 0x06, 0x06, 0xb6}, 0x012f},
	{"dsll",    {0x80, 0x80, 0x40, 0x40, 0x04, 0xb8}, 0x213f},
	{"dsrl",    {0x80, 0x80, 0x40, 0x40, 0x04, 0xba}, 0x213f},
	{"dsra",    {0x80, 0x80, 0x40, 0x40, 0x04, 0xbb}, 0x213f},
	{"dsll32",  {0x80, 0x80, 0x40, 0x40, 0x04, 0xbc}, 0x213f},
	{"dsrl32",  {0x80, 0x80, 0x40, 0x40, 0x04, 0xbe}, 0x213f},
	{"dsra32",  {0x80, 0x80, 0x40, 0x40, 0x04, 0xbf}, 0x213f},

	{"bltz",    {0x81, 0x40, 0x80, 0x02, 0x02, 0x02}, 0x02ff},
	{"bgez",    {0x81, 0x40, 0x81, 0x02, 0x02, 0x02}, 0x02ff},
	{"bltzl",   {0x81, 0x40, 0x82, 0x02, 0x02, 0x02}, 0x02ff},
	{"bgezl",   {0x81, 0x40, 0x83, 0x02, 0x02, 0x02}, 0x02ff},
	{"tgei",    {0x81, 0x40, 0x88, 0x00, 0x00, 0x00}, 0x02ff},
	{"tgeiu",   {0x81, 0x40, 0x89, 0x00, 0x00, 0x00}, 0x02ff},
	{"tlti",    {0x81, 0x40, 0x8a, 0x00, 0x00, 0x00}, 0x02ff},
	{"tltiu",   {0x81, 0x40, 0x8b, 0x00, 0x00, 0x00}, 0x02ff},
	{"teqi",    {0x81, 0x40, 0x8c, 0x00, 0x00, 0x00}, 0x02ff},
	{"tnei",    {0x81, 0x40, 0x8e, 0x00, 0x00, 0x00}, 0x02ff},
	{"bltzal",  {0x81, 0x40, 0x90, 0x02, 0x02, 0x02}, 0x02ff},
	{"bgezal",  {0x81, 0x40, 0x91, 0x02, 0x02, 0x02}, 0x02ff},
	{"bltzall", {0x81, 0x40, 0x92, 0x02, 0x02, 0x02}, 0x02ff},
	{"bgezall", {0x81, 0x40, 0x93, 0x02, 0x02, 0x02}, 0x02ff},

	{"j",       {0x82, 0x03, 0x03, 0x03, 0x03, 0x03}, 0x0fff},
	{"jal",     {0x83, 0x03, 0x03, 0x03, 0x03, 0x03}, 0x0fff},
	{"beq",     {0x84, 0x40, 0x40, 0x02, 0x02, 0x02}, 0x012f},
	{"bne",     {0x85, 0x40, 0x40, 0x02, 0x02, 0x02}, 0x012f},
	{"blez",    {0x86, 0x40, 0x80, 0x02, 0x02, 0x02}, 0x02ff},
	{"bgtz",    {0x87, 0x40, 0x80, 0x02, 0x02, 0x02}, 0x02ff},
	{"addi",    {0x88, 0x40, 0x40, 0x00, 0x00, 0x00}, 0x102f},
	{"addiu",   {0x89, 0x40, 0x40, 0x00, 0x00, 0x00}, 0x102f},
	{"slti",    {0x8a, 0x40, 0x40, 0x00, 0x00, 0x00}, 0x102f},
	{"sltiu",   {0x8b, 0x40, 0x40, 0x00, 0x00, 0x00}, 0x102f},
	{"andi",    {0x8c, 0x40, 0x40, 0x00, 0x00, 0x00}, 0x102f},
	{"ori",     {0x8d, 0x40, 0x40, 0x00, 0x00, 0x00}, 0x102f},
	{"xori",    {0x8e, 0x40, 0x40, 0x00, 0x00, 0x00}, 0x102f},
	{"lui",     {0x8f, 0x80, 0x40, 0x00, 0x00, 0x00}, 0x12ff},

	{"mfc0",    {0x90, 0x80, 0x40, 0x41, 0x80, 0x80}, 0x12ff},
	{"mtc0",    {0x90, 0x84, 0x40, 0x41, 0x80, 0x80}, 0x12ff},
	{"tlbr",    {0x90, 0x90, 0x80, 0x80, 0x80, 0x81}, 0xffff},
	{"tlbwi",   {0x90, 0x90, 0x80, 0x80, 0x80, 0x82}, 0xffff},
	{"tlbwr",   {0x90, 0x90, 0x80, 0x80, 0x80, 0x86}, 0xffff},
	{"tlbp",    {0x90, 0x90, 0x80, 0x80, 0x80, 0x88}, 0xffff},
	{"eret",    {0x90, 0x90, 0x80, 0x80, 0x80, 0x98}, 0xffff},

	{"mfc1",      {0x91, 0x80, 0x40, 0x42, 0x80, 0x80}, 0x12ff},
	{"dmfc1",     {0x91, 0x81, 0x40, 0x42, 0x80, 0x80}, 0x12ff},
	{"cfc1",      {0x91, 0x82, 0x40, 0x42, 0x80, 0x80}, 0x12ff},
	{"mtc1",      {0x91, 0x84, 0x40, 0x42, 0x80, 0x80}, 0x12ff},
	{"dmtc1",     {0x91, 0x85, 0x40, 0x42, 0x80, 0x80}, 0x12ff},
	{"ctc1",      {0x91, 0x86, 0x40, 0x42, 0x80, 0x80}, 0x12ff},
	{"bc1f",      {0x91, 0x88, 0x80, 0x02, 0x02, 0x02}, 0x2fff},
	{"bc1t",      {0x91, 0x88, 0x81, 0x02, 0x02, 0x02}, 0x2fff},
	{"bc1fl",     {0x91, 0x88, 0x82, 0x02, 0x02, 0x02}, 0x2fff},
	{"bc1tl",     {0x91, 0x88, 0x83, 0x02, 0x02, 0x02}, 0x2fff},
	{"add.s",     {0x91, 0x90, 0x42, 0x42, 0x42, 0x80}, 0x321f},
	{"add.d",     {0x91, 0x91, 0x42, 0x42, 0x42, 0x80}, 0x321f},
	{"sub.s",     {0x91, 0x90, 0x42, 0x42, 0x42, 0x81}, 0x321f},
	{"sub.d",     {0x91, 0x91, 0x42, 0x42, 0x42, 0x81}, 0x321f},
	{"mul.s",     {0x91, 0x90, 0x42, 0x42, 0x42, 0x82}, 0x321f},
	{"mul.d",     {0x91, 0x91, 0x42, 0x42, 0x42, 0x82}, 0x321f},
	{"div.s",     {0x91, 0x90, 0x42, 0x42, 0x42, 0x83}, 0x321f},
	{"div.d",     {0x91, 0x91, 0x42, 0x42, 0x42, 0x83}, 0x321f},
	{"sqrt.s",    {0x91, 0x90, 0x80, 0x42, 0x42, 0x84}, 0x32ff},
	{"sqrt.d",    {0x91, 0x91, 0x80, 0x42, 0x42, 0x84}, 0x32ff},
	{"abs.s",     {0x91, 0x90, 0x80, 0x42, 0x42, 0x85}, 0x32ff},
	{"abs.d",     {0x91, 0x91, 0x80, 0x42, 0x42, 0x85}, 0x32ff},
	{"mov.s",     {0x91, 0x90, 0x80, 0x42, 0x42, 0x86}, 0x32ff},
	{"mov.d",     {0x91, 0x91, 0x80, 0x42, 0x42, 0x86}, 0x32ff},
	{"neg.s",     {0x91, 0x90, 0x80, 0x42, 0x42, 0x87}, 0x32ff},
	{"neg.d",     {0x91, 0x91, 0x80, 0x42, 0x42, 0x87}, 0x32ff},
	{"round.l.s", {0x91, 0x90, 0x80, 0x42, 0x42, 0x88}, 0x32ff},
	{"round.l.d", {0x91, 0x91, 0x80, 0x42, 0x42, 0x88}, 0x32ff},
	{"trunc.l.s", {0x91, 0x90, 0x80, 0x42, 0x42, 0x89}, 0x32ff},
	{"trunc.l.d", {0x91, 0x91, 0x80, 0x42, 0x42, 0x89}, 0x32ff},
	{"ceil.l.s",  {0x91, 0x90, 0x80, 0x42, 0x42, 0x8a}, 0x32ff},
	{"ceil.l.d",  {0x91, 0x91, 0x80, 0x42, 0x42, 0x8a}, 0x32ff},
	{"floor.l.s", {0x91, 0x90, 0x80, 0x42, 0x42, 0x8b}, 0x32ff},
	{"floor.l.d", {0x91, 0x91, 0x80, 0x42, 0x42, 0x8b}, 0x32ff},
	{"round.w.s", {0x91, 0x90, 0x80, 0x42, 0x42, 0x8c}, 0x32ff},
	{"round.w.d", {0x91, 0x91, 0x80, 0x42, 0x42, 0x8c}, 0x32ff},
	{"trunc.w.s", {0x91, 0x90, 0x80, 0x42, 0x42, 0x8d}, 0x32ff},
	{"trunc.w.d", {0x91, 0x91, 0x80, 0x42, 0x42, 0x8d}, 0x32ff},
	{"ceil.w.s",  {0x91, 0x90, 0x80, 0x42, 0x42, 0x8e}, 0x32ff},
	{"ceil.w.d",  {0x91, 0x91, 0x80, 0x42, 0x42, 0x8e}, 0x32ff},
	{"floor.w.s", {0x91, 0x90, 0x80, 0x42, 0x42, 0x8f}, 0x32ff},
	{"floor.w.d", {0x91, 0x91, 0x80, 0x42, 0x42, 0x8f}, 0x32ff},
	{"cvt.s.d",   {0x91, 0x91, 0x80, 0x42, 0x42, 0xa0}, 0x32ff},
	{"cvt.s.w",   {0x91, 0x94, 0x80, 0x42, 0x42, 0xa0}, 0x32ff},
	{"cvt.s.l",   {0x91, 0x95, 0x80, 0x42, 0x42, 0xa0}, 0x32ff},
	{"cvt.d.s",   {0x91, 0x90, 0x80, 0x42, 0x42, 0xa1}, 0x32ff},
	{"cvt.d.w",   {0x91, 0x94, 0x80, 0x42, 0x42, 0xa1}, 0x32ff},
	{"cvt.d.l",   {0x91, 0x95, 0x80, 0x42, 0x42, 0xa1}, 0x32ff},
	{"cvt.w.s",   {0x91, 0x90, 0x80, 0x42, 0x42, 0xa4}, 0x32ff},
	{"cvt.w.d",   {0x91, 0x91, 0x80, 0x42, 0x42, 0xa4}, 0x32ff},
	{"cvt.l.s",   {0x91, 0x90, 0x80, 0x42, 0x42, 0xa5}, 0x32ff},
	{"cvt.l.d",   {0x91, 0x91, 0x80, 0x42, 0x42, 0xa5}, 0x32ff},
	{"c.f.s",     {0x91, 0x90, 0x42, 0x42, 0x80, 0xb0}, 0x21ff},
	{"c.f.d",     {0x91, 0x91, 0x42, 0x42, 0x80, 0xb0}, 0x21ff},
	{"c.un.s",    {0x91, 0x90, 0x42, 0x42, 0x80, 0xb1}, 0x21ff},
	{"c.un.d",    {0x91, 0x91, 0x42, 0x42, 0x80, 0xb1}, 0x21ff},
	{"c.eq.s",    {0x91, 0x90, 0x42, 0x42, 0x80, 0xb2}, 0x21ff},
	{"c.eq.d",    {0x91, 0x91, 0x42, 0x42, 0x80, 0xb2}, 0x21ff},
	{"c.ueq.s",   {0x91, 0x90, 0x42, 0x42, 0x80, 0xb3}, 0x21ff},
	{"c.ueq.d",   {0x91, 0x91, 0x42, 0x42, 0x80, 0xb3}, 0x21ff},
	{"c.olt.s",   {0x91, 0x90, 0x42, 0x42, 0x80, 0xb4}, 0x21ff},
	{"c.olt.d",   {0x91, 0x90, 0x42, 0x42, 0x80, 0xb4}, 0x21ff},
	{"c.ult.s",   {0x91, 0x91, 0x42, 0x42, 0x80, 0xb5}, 0x21ff},
	{"c.ult.d",   {0x91, 0x91, 0x42, 0x42, 0x80, 0xb5}, 0x21ff},
	{"c.ole.s",   {0x91, 0x90, 0x42, 0x42, 0x80, 0xb6}, 0x21ff},
	{"c.ole.d",   {0x91, 0x90, 0x42, 0x42, 0x80, 0xb6}, 0x21ff},
	{"c.ule.s",   {0x91, 0x91, 0x42, 0x42, 0x80, 0xb7}, 0x21ff},
	{"c.ule.d",   {0x91, 0x91, 0x42, 0x42, 0x80, 0xb7}, 0x21ff},
	{"c.sf.s",    {0x91, 0x90, 0x42, 0x42, 0x80, 0xb8}, 0x21ff},
	{"c.sf.d",    {0x91, 0x91, 0x42, 0x42, 0x80, 0xb8}, 0x21ff},
	{"c.ngle.s",  {0x91, 0x90, 0x42, 0x42, 0x80, 0xb9}, 0x21ff},
	{"c.ngle.d",  {0x91, 0x91, 0x42, 0x42, 0x80, 0xb9}, 0x21ff},
	{"c.seq.s",   {0x91, 0x90, 0x42, 0x42, 0x80, 0xba}, 0x21ff},
	{"c.seq.d",   {0x91, 0x91, 0x42, 0x42, 0x80, 0xba}, 0x21ff},
	{"c.ngl.s",   {0x91, 0x90, 0x42, 0x42, 0x80, 0xbb}, 0x21ff},
	{"c.ngl.d",   {0x91, 0x91, 0x42, 0x42, 0x80, 0xbb}, 0x21ff},
	{"c.lt.s",    {0x91, 0x90, 0x42, 0x42, 0x80, 0xbc}, 0x21ff},
	{"c.lt.d",    {0x91, 0x91, 0x42, 0x42, 0x80, 0xbc}, 0x21ff},
	{"c.nge.s",   {0x91, 0x90, 0x42, 0x42, 0x80, 0xbd}, 0x21ff},
	{"c.nge.d",   {0x91, 0x91, 0x42, 0x42, 0x80, 0xbd}, 0x21ff},
	{"c.le.s",    {0x91, 0x90, 0x42, 0x42, 0x80, 0xbe}, 0x21ff},
	{"c.le.d",    {0x91, 0x91, 0x42, 0x42, 0x80, 0xbe}, 0x21ff},
	{"c.ngt.s",   {0x91, 0x90, 0x42, 0x42, 0x80, 0xbf}, 0x21ff},
	{"c.ngt.d",   {0x91, 0x91, 0x42, 0x42, 0x80, 0xbf}, 0x21ff},

	{"beql",    {0x94, 0x40, 0x40, 0x02, 0x02, 0x02}, 0x012f},
	{"bnel",    {0x95, 0x40, 0x40, 0x02, 0x02, 0x02}, 0x012f},
	{"blezl",   {0x96, 0x40, 0x80, 0x02, 0x02, 0x02}, 0x02ff},
	{"bgtzl",   {0x97, 0x40, 0x80, 0x02, 0x02, 0x02}, 0x02ff},
	{"daddi",   {0x98, 0x40, 0x40, 0x00, 0x00, 0x00}, 0x102f},
	{"daddiu",  {0x99, 0x40, 0x40, 0x00, 0x00, 0x00}, 0x102f},
	{"ldl",     {0x9a, 0x40, 0x40, 0x01, 0x01, 0x01}, 0x120f},
	{"ldr",     {0x9b, 0x40, 0x40, 0x01, 0x01, 0x01}, 0x120f},
	{"lb",      {0xa0, 0x40, 0x40, 0x01, 0x01, 0x01}, 0x120f},
	{"lh",      {0xa1, 0x40, 0x40, 0x01, 0x01, 0x01}, 0x120f},
	{"lwl",     {0xa2, 0x40, 0x40, 0x01, 0x01, 0x01}, 0x120f},
	{"lw",      {0xa3, 0x40, 0x40, 0x01, 0x01, 0x01}, 0x120f},
	{"lbu",     {0xa4, 0x40, 0x40, 0x01, 0x01, 0x01}, 0x120f},
	{"lhu",     {0xa5, 0x40, 0x40, 0x01, 0x01, 0x01}, 0x120f},
	{"lwr",     {0xa6, 0x40, 0x40, 0x01, 0x01, 0x01}, 0x120f},
	{"lwu",     {0xa7, 0x40, 0x40, 0x01, 0x01, 0x01}, 0x120f},
	{"sb",      {0xa8, 0x40, 0x40, 0x01, 0x01, 0x01}, 0x120f},
	{"sh",      {0xa9, 0x40, 0x40, 0x01, 0x01, 0x01}, 0x120f},
	{"swl",     {0xaa, 0x40, 0x40, 0x01, 0x01, 0x01}, 0x120f},
	{"sw",      {0xab, 0x40, 0x40, 0x01, 0x01, 0x01}, 0x120f},
	{"sdl",     {0xac, 0x40, 0x40, 0x01, 0x01, 0x01}, 0x120f},
	{"sdr",     {0xad, 0x40, 0x40, 0x01, 0x01, 0x01}, 0x120f},
	{"swr",     {0xae, 0x40, 0x40, 0x01, 0x01, 0x01}, 0x120f},
	{"cache",   {0xaf, 0x40, 0x05, 0x01, 0x01, 0x01}, 0x120f},
	{"ll",      {0xb0, 0x40, 0x40, 0x01, 0x01, 0x01}, 0x120f},
	{"lwc1",    {0xb1, 0x40, 0x42, 0x01, 0x01, 0x01}, 0x120f},
	{"lld",     {0xb4, 0x40, 0x40, 0x01, 0x01, 0x01}, 0x120f},
	{"ldc1",    {0xb5, 0x40, 0x42, 0x01, 0x01, 0x01}, 0x120f},
	{"ld",      {0xb7, 0x40, 0x40, 0x01, 0x01, 0x01}, 0x120f},
	{"sc",      {0xb8, 0x40, 0x40, 0x01, 0x01, 0x01}, 0x120f},
	{"swc1",    {0xb9, 0x40, 0x42, 0x01, 0x01, 0x01}, 0x120f},
	{"scd",     {0xbc, 0x40, 0x40, 0x01, 0x01, 0x01}, 0x120f},
	{"sdc1",    {0xbd, 0x40, 0x42, 0x01, 0x01, 0x01}, 0x120f},
	{"sd",      {0xbf, 0x40, 0x40, 0x01, 0x01, 0x01}, 0x120f}
};

#endif