#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

typedef unsigned char byte;
typedef int word;
typedef int adr;

#define LO(x) ((x) & 0xFF)
#define HI(x) ((x >> 8) & 0xFF)

word mem[56*1024];

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
//void load_file(const char * filename){
void load_file(const char * file){
	FILE *f_in;
	f_in = fopen(file, "r");
	adr a;
	unsigned int b;
	int i;
	int n;
	while(fscanf(f_in,"%x%x", &a, &n) == 2) {
		 for (i = 0; i < n ; i++) {
			fscanf(f_in,"%x", &b);
			b_write(a+i, (byte)b);
		}
	}
	fclose(f_in);
}
void mem_dump(adr start, word n) {
	int i;
	for(i = 0; i < n; i = i + 2) {
		printf("%06o : %06o\n", start + i, w_read(start + i));
	}
}

void do_halt() {
	printf("The End!!!\n");
	exit(0);
}

void do_add() {
	return;
}

void do_move() {
	return;
}

void do_unknown() {
	return;
}

struct Command {
	word opcode;
	word mask;
	const char * name;
	void (*do_func)();
} command[] = {
	{0010000, 0170000, "mov", do_move},
	{0060000, 0170000, "add", do_add},
	{0000000, 0177777, "halt", do_halt},
	{0000000, 0170000, "unknown", do_unknown},
};
/*void run() {
	word pc = 01000;
	while(1) {
		word w = w_read(pc);
		fprintf(stdout, "%06o: %06o ", pc, w);
		pc += 2;
		for (i = 0; i < ; i ++){
			cmd = command[i];
			if ()
}*/
int main() {
 //   byte b0;
 //  byte  b1;
 //  word w;
 FILE * file;
 file = fopen("pdp.exe", 'r');
 
	load_file(pdp.exe);
	mem_dump(0x40, 12);
	mem_dump(0x52, 12);
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
