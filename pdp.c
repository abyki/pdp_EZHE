#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>

//#define LO(x) (x & 0xFF)
//#define HI(x) ((x >> 8) & 0xFF)

#define NO_PARAM 0
#define HAS_XX 1
#define HAS_SS (1<<1)
#define HAS_DD (1<<2)
#define HAS_NN (1<<3)
#define HAS_R4 (1<<4)

#define RELEASE 0
#define DEBUG 1
#define FULL_DEBUG 2

int debug_level = DEBUG;

typedef unsigned int byte;
typedef int word;
typedef word adr;

byte mem[64*1024];

word reg[8];
#define sp reg[6]
#define pc reg[7]

void b_write(adr a, byte x);
byte b_read(adr a);
void w_write(adr a, word x);
word w_read(adr a);
void test_mem();
void trace(int dbg_lvl, char * format, ...);
void mem_dump(adr start, word n);
void print_reg();
void do_halt();
void do_add();
void do_mov();
void do_unknown();
void load_file();
void run();
struct SSDD;
struct Command;
struct SSDD get_mode(word w);

struct SSDD {
	word val;
	adr a;
};

 int R4, nn, BYTE;

struct SSDD ss, dd;

void b_write(adr a, byte x) {
	if (a < 8) {
		reg[a] = (x >> 7) ? (x | 0xFF00) : x;
	}
	else {
		mem[a] = x;
	}
}
byte b_read(adr a) {
	byte x;
	x = (a > 7) ? (mem[a]) : (reg[a]);
	return x;
}
void w_write(adr a, word x) {
	if  (a < 8) {
		reg[a] = x;
	}
	else 
	{
		assert(!(a % 2));
	    mem[a] = x & 0xFF;;
	    mem[a+1] = (x >> 8) & 0xFF;
	}
}

word w_read(adr a) {
	word x = 0;
    if (a > 7) {
		assert(!(a % 2));
	}
	x = (a > 7) ? (((x | mem[a + 1]) << 8) | mem[a]) : reg[a];
	return x;
		
}
void test_mem(){
	b_write(2, 0x0a);
	b_write(3, 0x0b);
	word w = w_read(2);
	printf("0b0a = %hx\n", w);
	w_write(4, 0x0d0c);
	byte b4 = b_read(4);
	byte b5 = b_read(5);
	printf("0d0c = %hx %hx\n", b5, b4);
}

void trace(int dbg_lvl, char * format, ...) {
	if (dbg_lvl != debug_level)
		return;
	va_list ap;
	va_start (ap, format);
	vprintf (format, ap);
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
		printf("R%d : %.6o ", i, reg[i]);
		reg[i] = 0;
	}
}
void do_halt() {
	printf("\n");
	print_reg();
	printf("The End!\n");
	exit(0);
} 

void do_add() {
	w_write(dd.a, ((dd.val + ss.val) & 0xFFFF));
	return;
}

void do_mov() {
	w_write(dd.a, (ss.val & 0xFFFF));
	return;
}

void do_sob() {
	reg[R4]--;
	if (reg[R4] != 0)
		pc = (pc - (2 * nn)) & 0xFFFF;
	printf("R%d\n", R4);
}

void do_clr() {
	w_write(dd.a, 0);
}

void do_unknown() {
	printf("WHAT A MISTAKE, DUDE?!\n");
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
		int res = fscanf(f_in, "%x%x", &adress, &n);
		if (2 != res)
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
	short int is_it_byte;
	const char * name;
	void (*do_func)();
	byte param;
};
struct Command command[] = {
	{0010000, 0170000, 0, "mov",      do_mov,     HAS_SS | HAS_DD},
	{0060000, 0170000, 1, "add",      do_add,     HAS_SS | HAS_DD},
	{0077000, 0177000, 0, "sob",      do_sob,     HAS_R4 | HAS_NN},
	{0005000, 0177700, 0, "clr",      do_clr,     HAS_DD},
	{0000000, 0177777, 0, "halt",     do_halt,    NO_PARAM},
	{0000000, 0170000, 0, "unknown",  do_unknown, NO_PARAM}
};

struct SSDD get_mode (word w) {
	struct SSDD result;
	int n = w & 7;
	int mode = (w>>3) & 7;
	switch(mode) {
		case 0:
					result.a = n;
					result.val = reg[n];
					printf("R%d ", n);
					break;
		case 1:
					result.a = reg[n];
					if (BYTE) {
						result.val = b_read(result.a);
					}
					else
					{
						result.val = w_read(result.a);
					}
					printf("(R%d) ", n);
					break;
		case 2:
					result.a = reg[n];
					if ((BYTE)&&(n!=6)&&(n!=7)) {
						result.val = b_read(result.a);
					}
					else
					{
						result.val = w_read(result.a);
					}
					if (n != 7) {
						printf ("(R%d)+ ", n);
					}
					else {
						printf(" #%o ", result.val);
					}
					reg[n] = ((BYTE)&&(n!=6)&&(n!=7)) ? (reg[n] + 1) : (reg[n] + 2);
					break;
		case 3:
					result.a = w_read(reg[n]);
					if (BYTE) {
						result.val = b_read(result.a);
					}
					else
					{
						result.val = w_read(result.a);
					}
					if (n != 7) {
						printf ("@(R%d)+ ", n);
					}
					else {
						printf(" @#%o ", result.val);
					}
					reg[n] += 2;
					break;
		case 4:
					reg[n] = ((BYTE)&&(n!=6)&&(n!=7)) ? (reg[n] - 1) : (reg[n] - 2);
					result.a = reg[n];
					if (BYTE) {
						result.val = b_read(result.a);
					}
					else
					{
						result.val = w_read(result.a);
					}
				    printf ("-(R%d)+ ", n);
					break;
	    case 5: 
	                reg[n] -= 2;
	                result.a = w_read(reg[n]);
	                if (BYTE) {
						result.val = b_read(result.a);
					}
					else
					{
						result.val = w_read(result.a);
					}
	                printf("@-(R%d) ", n);
	                break;    
	    case 6: 
		//            nn = w_read(pc);
		  //          pc += 2;
		            
		            
		            
		
		default: 
					printf("I dont know T.T, I'm so stupid((( \n");
					exit(2);
		}
		return result;
}

void run() {
	printf("\n");
	printf("Start f-on run\n");
	printf("\n");
	pc = 01000;
	while(1) {
		word w = w_read(pc) & 0xFFFF;
		fprintf(stdout, "%06o: %06o ", pc, w);
		pc += 2;
		for (int i = 0; i < 4; i ++){
			struct Command cmd = command[i];
			if ((w & cmd.mask) == cmd.opcode) {
				printf("%s", cmd.name);
				printf(" ");
				if (cmd.param & HAS_SS)
					ss = get_mode (w>>6);
				if (cmd.param & HAS_DD)
					dd = get_mode(w);
				if (cmd.param & HAS_R4)
					R4 = (w >> 6) & 7;
				if (cmd.param & HAS_NN)
					nn = w & 63;	
				printf("\n");
				cmd.do_func();
				printf("\n");
				break;
			}
		}
	}
}


int main() {
	test_mem();
	load_file();
	mem_dump(0x200, 0xc);
	run();
	
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
