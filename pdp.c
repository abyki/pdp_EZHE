#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>

#define LO(x) ((x) & 0xFF)
#define HI(x) ((x >> 8) & 0xFF)\

#define NO_PARAM 0
#define HAS_XX 1
#define HAS_SS (1<<1)
#define HAS_DD (1<<2)

#define RELEASE 0
#define DEBUG 1
#define FULL_DEBUG 2

int debug_level = DEBUG;

typedef unsigned char byte;
typedef int word;
typedef int adr;

word mem[64*1024];

word reg[8];
#define sp reg[6];
#define pc reg[7];

struct SSDD {
	word val;
	adr a;
} ss, dd;

void b_write(adr a, byte x) {
	mem[a] = x;
}
byte b_read(adr a) {
	return mem[a];
}
void w_write(adr a, word x) {
	assert(a % 2 == 0);
	mem[a] = LO(x);
	mem[a+1] = HI(x);
}

word w_read(adr a) {
	word res;
	assert(a % 2 == 0);
	res = (word)(mem[a]) | (word)(mem[a+1] << 8);
	return res;
		
}
void test_mem(byte b0, byte  b1, word w){
	assert(b0 == LO(w));
	assert(b1 == HI(w));
}

void trace(int dbg_lvl, char * format, ...) {
	if (dbg_lvl != debug_level)
		return;
	va_list ap;
	va_start(ap, format);
	vprintf(format, ap);
	va_end(ap);
}


void mem_dump(adr start, word n) {
	int i;
	for(i = 0; i < n; i = i + 2) {
		word res = w_read(start + i);
		trace(debug_level, "%06o : %06o\n", start + i, res);
	}
}


void print_reg() {
	for (int i = 0; i < 8; i++) {
		printf("%d : %.6o ", i, reg[i]);
	}

void do_halt() {
	print_reg();
	printf("The End!!!\n");
	exit(0);
}

void do_add() {
	w_write(dd.a, ss.val + ss.val);
	return;
}

void do_move() {
	w_write(dd.a, ss.val);
	return;
}

void do_unknown() {
	printf("WHAT A MISTAKE, DUDE?!");
	print_reg();
	return;
}

void load_file() {
	FILE *f_in = NULL;
	f_in = stdin;
	if (f_in == NULL) {
		perror("in.txt"); //pechataet oshibky
		exit(1);
	}
	unsigned int adress, n;
	int i;
	while (1) {
		if (1 != fscanf(f_in, "%x%x", &adress, &n))
			return;
		for (i = 0; i < n; i++) {
			unsigned int x;
			fscanf (f_in, "%x", &x);
			b_write(adress + i, (byte)x);
		}
	}
	fclose (f_in);
}

struct Command {
	word opcode;
	word mask;
	const char * name;
	void (*do_func)();
	byte param;
} command[] = {
	{0010000, 0170000, "mov", do_move, HAS_SS | HAS_DD},
	{0060000, 0170000, "add", do_add, HAS_SS | HAS_DD},
	{0000000, 0177777, "halt", do_halt, NO_PARAM},
	{0000000, 0170000, "unknown", do_unknown, NO_PARAM},
}
/*
void run() {
	word pc = 01000;
	while(1) {
		word w = w_read(pc);
		fprintf(stdout, "%06o: %06o ", pc, w);
		pc += 2;
		for (int i = 0; i < 4; i ++){
			cmd = command[i];
			if ((w & cmd.mask) == cmd.opcode) {
				printf("%s", cmd.name);
				printf(" ");
				if (cmd.param & HAS_SS)
					ss = get_mode (w>>6);
				if (cmd.param & HAS_DD)
					dd = get_mode(w);
				printf("\n");
				cmd.do_func();
				print_reg();
				break;
			}
}
*/
int main() {
	load_file();
	mem_dump(0x200, 0xc);
/*
    w = 0x0b0a;
	b0 = 0xa;
	b1 = 0xb;
	b_write(0, b0);
	b_write(1, b1);
	w = w_read(0); //0x0b0a
	test_mem(b0, b1, w);
	printf("%04x = %02hhx%02hhx\n", w, b1, b0); // 0b0a = 0b0a

    w = 0x0b0a;
    w_write(2, w);
    b0 = 0x0a;
	b1 = 0x0b;
	w = 0x0a0b;
	b_write(0, b0);
	b_write(1, b1);
	w = w_read(0); //0x0b0a
	printf("%04x = %02hhx%02hhx\n", w, b1, b0); // 0b0a = 0b0a
	
	test_mem(b0, b1, w);
*/

	return 0;
}
