#include <stdlib.h> 
#include <stdint.h>
#include <stdio.h>
#include <time.h>



typedef struct {
	int size;
	char *content;
} array;


int STACK_SIZE = 8;
int id = 0;
typedef struct {
	int parent_id;
	int id;
	//location in memory
	int startx;
	int starty;
	//size of body's memory chunk
	int width;
	int height;
	//instruction pointer (!not absolute: relative position in the body)
	int ptrx;
	int ptry;
	//direction in which we currently readcode
	int deltax;
	int deltay;
	//Registers
	//register a
	int ax;
	int ay;
	//register b
	int bx;
	int by;
	//register c
	int cx;
	int cy;
	//register d
	int dx;
	int dy;
	//stack
	int stackx[8];
	int stacky[8];
	int stacktop;
	int errors;
	// Reproduction
	int reproduction_cycle; //gets reset to 0 every time organism reproduces
	// child
	int childx;
	int childy;
	int child_width;
	int child_height;
	int children;
} organism;

typedef struct {
	int top;
	int size;
	int* organisms_order;
	organism* organisms;
} table_of_organisms;


void table_create(table_of_organisms* table, int size) {
	table->top = 0;
	table->size = size;
	table->organisms_order = calloc(size, sizeof(int));
	table->organisms = calloc(size, sizeof(organism*));
}

void print_queue(table_of_organisms* table) {
	for (int j = 0; j < table->top; j++) {
		printf("%d id %d at (%d %d) errors %d\n", j, table->organisms[j].id, table->organisms[j].startx, table->organisms[j].starty, table->organisms[j].errors);
	}
}

int kill_organism(organism org, array* memory);

int MAX_N_OF_ERRORS = 5000;
int MAX_ITERS_WITHOUT_REPRODUCTION = 100000;
void update_queue(table_of_organisms* table, array* memory) {
	// Sort organisms by number of errors (in ascending order)
	// insertion sort
	int temp;
	for (int j = 1; j < table->top; j++) {
		for (int i = j; i > 0; i--) {
			if (table->organisms[table->organisms_order[i]].errors < table->organisms[table->organisms_order[i-1]].errors) {
				// swipe
				temp = table->organisms_order[i];
				table->organisms_order[i] = table->organisms_order[i-1];
				table->organisms_order[i-1] = temp;
			}
		}
	}
	int end = table->top;
	for (int i = table->top-1; i >= 0; i--) {
		if ((table->organisms[i].errors) > MAX_N_OF_ERRORS) {
			end = i;
			break;
		}
	}
	// Kill organisms that made too many errors
	for (int i = end; i < table->top; i++) {
		if (!(kill_organism(table->organisms[table->organisms_order[i]], memory) == 0)) {
			return;
		}
	}
	// update table size
	table->top = end;
}

int append_queue(table_of_organisms* table, organism* org) {
	// add an organism in the topmost free position of table
	if (table->top == table->size) {
		return -1;
	}
	table->organisms_order[table->top] = table->top;
	table->organisms[table->top] = *org;
	table->top++;
	printf("Q %d %d %d %d %d %d|", org->id, org->parent_id, org->startx, org->starty, org->width, org->height);
	return 0;
}

// COMMANDS
char REGISTER_A = 'a';
char REGISTER_B = 'b';
char REGISTER_C = 'c';
char REGISTER_D = 'd';
char commands[25] = {'.', ':', 'a', 'b', 'c', 'd', 'x', 'y', '^', 'v', '>', '<', '&', '?', '1', '0', '-', '+', '~', 'L', 'W', '@', '$', 'S', 'P'};
int N_OF_COMMANDS = 25;


int command_symbol_to_code(char s) {
	for (int i = 0; i < N_OF_COMMANDS; i++) {
		if (s == commands[i]) {
			return i;
		}
	}
	return -1;
}

int COMMAND_LENGTH = 5;
/*
'.' : 000001 : 'no_operation',
':' : 000010 : 'no_operation',
'a' : 000011 : 'no_operation',
'b' : 000100 : 'no_operation',
'c' : 000101 : 'no_operation',
'd' : 000110 : 'no_operation',
'x' : 000111 : 'no_operation',
'y' : 001000 : 'no_operation',
'^' : 001001 : 'move_organism_pointer_up',
'v' : 001010 : 'move_organism_pointer_down',
'>' : 001011 : 'move_organism_pointer_right',
'<' : 001100 : 'move_organism_pointer_left',
'&' : 001101 : 'find_template',
'?' : 001110 : 'if_not_zero',
'1' : 001111 : 'one',
'0' : 010000 : 'zero',
'-' : 010001 : 'decrement',
'+' : 010010 : 'increment',
'~' : 010011 : 'subtract',
'L' : 010100 : 'load_inst',
'W' : 010101 : 'write_inst',
'@' : 010110 : 'allocate_child',
'$' : 010111 : 'split_child',
'S' : 011000 : 'push',
'P' : 011001 : 'pop',
*/

// MEMORY
/*
free :      00000000
allocated : 00000001
active :    00000010
*/

int set_register_value(organism* org, int register_code, int x, int y) {
	if ((register_code < 0) || (register_code >= N_OF_COMMANDS)) {
		//printf("unknown command code %d\n", register_code);
		return -1;
	}
	char reg = commands[register_code];
	if (reg == REGISTER_A) {
		org->ax = x;
		org->ay = y;
	}
	else if (reg == REGISTER_B) {
		org->bx = x;
		org->by = y;
	}
	else if (reg == REGISTER_C) {
		org->cx = x;
		org->cy = y;
	}
	else if (reg == REGISTER_D) {
		org->dx = x;
		org->dy = y;
	}
	else {
		// register doesn't exist
		return -1;
	}
	return 0;
}

int get_register_value(organism* org, int register_code, int* x, int* y) {
	if ((register_code < 0) || (register_code >= N_OF_COMMANDS)) {
		// unknown command code
		return -1;
	}
	char reg = commands[register_code];
	if (reg == REGISTER_A) {
		*x = org->ax;
		*y = org->ay;
	}
	else if (reg == REGISTER_B) {
		*x = org->bx;
		*y = org->by;
	}
	else if (reg == REGISTER_C) {
		*x = org->cx;
		*y = org->cy;
	}
	else if (reg == REGISTER_D) {
		*x = org->dx;
		*y = org->dy;
	}
	else {
		// register doesn't exist
		return -1;
	}
	return 0;
}

int organism_create(organism* org, int startx, int starty, int width, int height, int parent_id, table_of_organisms* table) {
	if (!org) {
		return -1;
	}
	org->parent_id = parent_id; //remember ancestor
	org->id = id;
	id++;
	org->startx = startx;
	org->starty = starty;
	org->width = width;
	org->height = height;
	//!!! absolute position of the pointer
	org->ptrx = 0;
	org->ptry = 0;
	org->deltax = 1;
	org->deltay = 0;
	org->stacktop = 0;
	// registers
	org->ax = 0;
	org->ay = 0;
	org->bx = 0;
	org->by = 0;
	org->cx = 0;
	org->cy = 0;
	org->dx = 0;
	org->dy = 0;
	// reproduction
	org->errors = 0;
	org->reproduction_cycle = 0;
	// child
	org->child_height = 0;
	org->child_width = 0;
	org->childx = 0;
	org->childy = 0;
	org->children = 0;
	append_queue(table, org);
	return 0;
}


void move_organism_pointer(organism* org) {
	org->ptrx += org->deltax;
	org->ptry += org->deltay;
}

void print_array(array* arr) {
	// print command symbols in memory
	for (int i = 0; i < arr->size; i++) {
		for (int j = 0; j < arr->size; j++) {
			printf("%c", commands[arr->content[i*arr->size + j] >> 2]);
		}
		printf("\n");
	}
	printf("\n");
}

void print_array_mask(array* arr) {
	// print types of memory cells (free, allocated, active)
	for (int i = 0; i < arr->size; i++) {
		for (int j = 0; j < arr->size; j++) {
			printf("%d", 3 & arr->content[i*arr->size + j]);
		}
		printf("\n");
	}
	printf("\n");
}


int array_create(array* arr, int size) {
	arr->size = size;
	arr->content = calloc(size*size, sizeof(char));
	if (!arr->content) {
		return -1;
	}
	return 0;
}

int does_fit(array* memory, int x, int y, int w, int h) {
	//check wether chunk (x, y) (x+w, y+h) is withing 2d array matrix
	return ((x >= 0) && (x < memory->size) && 
		(x+w >= 0) && (x+w < memory->size) &&
		(y >= 0) && (y < memory->size) &&
		(y+h >= 0) && (y+h < memory->size));
}

int memory_is_free(array* memory, int x, int y, int w, int h) {
	if (!does_fit(memory, x, y, w, h)) {
		return 0;
	}
	for (int i = x; i < x+w; i++) {
		for (int j = y; j < y+h; j++) {
			if (!((3 & memory->content[i*memory->size+j]) == 0)) {
				return 0;
			}
		}
	}
	return 1;
}

int memory_is_active(array* memory, int x, int y, int w, int h) {
	if (!does_fit(memory, x, y, w, h)) {
		return 0;
	}
	for (int i = x; i < x+w; i++) {
		for (int j = y; j < y+h; j++) {
			if (!((3 & memory->content[i*memory->size+j]) == 2)) {
				return 0;
			}
		}
	}
	return 1;
}

int memory_allocate(array* memory, int x, int y, int w, int h) {
	if (!does_fit(memory, x, y, w, h)) {
		return -1;
	}
	if (!memory_is_free(memory, x, y, w, h)) {
		return -1;
	}
	for (int i = x; i < x+w; i++) {
		for (int j = y; j < y+h; j++) {
			memory->content[i*memory->size+j] ^= 3 & memory->content[i*memory->size+j];
			memory->content[i*memory->size+j] |= 1;
		}
	}
	printf("ML %d %d %d %d|", x, y, w, h);
	return 0;
}

int memory_activate(array* memory, int x, int y, int w, int h) {
	if (!does_fit(memory, x, y, w, h)) {
		return -1;
	}
	if (memory_is_active(memory, x, y, w, h)) {
		return -1;
	}
	for (int i = x; i < x+w; i++) {
		for (int j = y; j < y+h; j++) {
			memory->content[i*memory->size+j] ^= 3 & memory->content[i*memory->size+j];
			memory->content[i*memory->size+j] |= 2;
		}
	}
	printf("MA %d %d %d %d|", x, y, w, h);
	return 0;
}

int memory_free(array* memory, int x, int y, int w, int h) {
	if (!does_fit(memory, x, y, w, h)) {
		return 0;
	}
	for (int i = x; i < x+w; i++) {
		for (int j = y; j < y+h; j++) {
			memory->content[i*memory->size+j] ^= 3 & memory->content[i*memory->size+j];
		}
	}
	printf("MF %d %d %d %d|", x, y, w, h);
	return 0;
}

char array_get(array* arr, int x, int y) {
	// get byte (char) in position (x, y)
	if ((x < 0) || (x >= arr->size) || (y < 0) || (y >= arr->size)) {
		return -1;
	}
	int index = x*arr->size + y;
	return arr->content[index];
}

int array_get_command_code(array* arr, int x, int y) {
	if ((x < 0) || (x >= arr->size) || (y < 0) || (y >= arr->size)) {
		return -1;
	}
	int index = x*arr->size + y;
	return (arr->content[index])>>2;
}

int array_get_cell_type(array* arr, int x, int y) {
	if ((x < 0) || (x >= arr->size) || (y < 0) || (y >= arr->size)) {
		return -1;
	}
	int index = x*arr->size + y;
	return 3 & arr->content[index];
}

int array_set(array* arr, int x, int y, char c) {
	// set value of byte in position (x, y)
	if ((x < 0) || (x >= arr->size) || (y < 0) || (y >= arr->size)) {
		return -1;
	}
	int command_code = c >> 2;
	if ((command_code < 0) || (command_code >= N_OF_COMMANDS)) {
		return -1;
	}
	int cell_type = 3 & c;
	if ((cell_type < 0) || (cell_type > 2)) {
		return -1;
	}
	int index = x*arr->size + y;
	arr->content[index] = c;
	printf("MS %d %d %d %d|", x, y, command_code, cell_type);
	return 0;
}

int array_set_command(array* arr, int x, int y, int code) {
	if ((x < 0) || (x >= arr->size) || (y < 0) || (y >= arr->size)) {
		return -1;
	}
	if ((code < 0) || (code >= N_OF_COMMANDS)) {
		return -1;
	}
	int index = x*arr->size + y;
	arr->content[index] = (code << 2) + (3 & arr->content[index]);
	printf("MW %d %d %c|", x, y, commands[code]);
	return 0;
}


int kill_organism(organism org, array* memory) {
	if (!(memory_free(memory, org.startx, org.starty, org.width, org.height) == 0)) {
		return -1;
	}
	return 0;
}

int left(organism* org) {
	if (!org) {
		return -1;
	}
	org->deltax = 0;
	org->deltay = -1;
	return 0;
}

int right(organism* org) {
	if (!org) {
		return -1;
	}
	org->deltax = 0;
	org->deltay = 1;
	return 0;
}

int up(organism* org) {
	if (!org) {
		return -1;
	}
	org->deltax = -1;
	org->deltay = 0;
	return 0;
}

int down(organism* org) {
	if (!org) {
		return -1;
	}
	org->deltax = 1;
	org->deltay = 0;
	return 0;
}

int Stack(array* memory, organism* org) {
	// push value of register into stack
	char reg = array_get_command_code(memory, org->ptrx + org->startx+org->deltax, org->ptry + org->starty+org->deltay);
	int x;
	int y;
	if (!(get_register_value(org, reg, &x, &y) == 0)) {
		return -1;
	}
	if (org->stacktop >= STACK_SIZE) {
		return -1;
	}
	org->stackx[org->stacktop] = x;
	org->stacky[org->stacktop] = y;
	org->stacktop++;
	return 0;
}

int Pop(array* memory, organism* org) {
	// pop value from stack into register
	if (org->stacktop == 0) {
		return -1;
	}
	int x = org->stackx[org->stacktop-1];
	int y = org->stacky[org->stacktop-1];
	int reg = array_get_command_code(memory, org->ptrx+org->startx+org->deltax, org->ptry+org->starty+org->deltay);
	if (!(set_register_value(org, reg, x, y) == 0)) {
		return -1;
	}
	org->stacktop--;
	return 0;
}

int jumpc(array* memory, organism* org) {
	/*go to closer option if zero, to further one if not zero

	|<-ptr	  a: (0, something)
	|		  on the next step: >
	?xa>^

	|<-ptr	  a: (0, 0)
	|		  on the next step: >
	?a>^
	
	|<-ptr   a: NOT (0, 0)
	|		 on the next step: ^
	?a>^

	|<-ptr	 a: (something, anything)
	|		 on the next step: ^
	?xa>^
	*/
	int reg = array_get_command_code(memory, org->ptrx + org->startx+org->deltax, org->ptry + org->starty+org->deltay);
	if ((reg < 0) || (reg >= N_OF_COMMANDS)) {
		return -1;
	}
	int x = 0;
	int y = 0;
	int xs;
	int ys;
	if ((commands[reg] == 'x') || (commands[reg] == 'y')) {
		reg = commands[reg];
		x = (reg == 'x') ? 1 : 0;
		y = (reg == 'y') ? 1 : 0;
		reg = array_get_command_code(memory, org->ptrx + org->startx+org->deltax*2, org->ptry + org->starty+org->deltay*2);
		move_organism_pointer(org);
	}
	if (!(get_register_value(org, reg, &xs, &ys) == 0)) {
		return -1;
	}
	if (((x == 0) && (y == 0) && (xs == 0) && (ys == 0)) ||
	((x == 1) && (xs == 0)) || ((y == 1) && (ys == 0))) {
		move_organism_pointer(org);
	}
	else {
		move_organism_pointer(org);
		move_organism_pointer(org);
	}
	return 0;
}

int increment(array* memory, organism* org) {
	//increment a component \ both components of a register
	int reg = array_get_command_code(memory, org->ptrx + org->startx+org->deltax, org->ptry + org->starty+org->deltay);
	if ((reg < 0) || (reg >= N_OF_COMMANDS)) {
		return -1;
	}
	int x = 1;
	int y = 1;
	int xs;
	int ys;
	if ((commands[reg] == 'x') || (commands[reg] == 'y')) {
		//partial increment: increment value of one of register's components
		reg = commands[reg];
		x = (reg == 'x') ? 1 : 0;
		y = (reg == 'y') ? 1 : 0;
		reg = array_get_command_code(memory, org->ptrx + org->startx+org->deltax*2, org->ptry + org->starty+org->deltay*2);
	}
	if (!(get_register_value(org, reg, &xs, &ys) == 0)) {
		return -1;
	}
	if (!(set_register_value(org, reg, xs+x, ys+y) == 0)) {
		return -1;
	}
	return 0;
}

int decrement(array* memory, organism* org) {
	//decrement a component \ both components of a register
	int reg = array_get_command_code(memory, org->ptrx + org->startx+org->deltax, org->ptry + org->starty+org->deltay);
	if ((reg < 0) || (reg >= N_OF_COMMANDS)) {
		return -1;
	}
	int x = 1;
	int y = 1;
	int xs;
	int ys;
	if ((commands[reg] == 'x') || (commands[reg] == 'y')) {
		//partial decrement: increment value of one of register's components
		reg = commands[reg];
		x = (reg == 'x') ? 1 : 0;
		y = (reg == 'y') ? 1 : 0;
		reg = array_get_command_code(memory, org->ptrx + org->startx+org->deltax*2, org->ptry + org->starty+org->deltay*2);
	}
	if (!(get_register_value(org, reg, &xs, &ys) == 0)) {
		return -1;
	}
	if (!(set_register_value(org, reg, xs-x, ys-y) == 0)) {
		return -1;
	}
	return 0;
}

int write_instance(array* memory, organism* org) {
	// write instance from one register to another
	int regto = array_get_command_code(memory, org->ptrx + org->startx+org->deltax, org->ptry + org->starty+org->deltay);
	int regfrom = array_get_command_code(memory, org->ptrx + org->startx+org->deltax*2, org->ptry + org->starty+org->deltay*2);
	int addressx;
	int addressy;
	int x;
	int y;
	if (!(get_register_value(org, regfrom, &x, &y) == 0)) {
		return -1;
	}
	if (!(get_register_value(org, regto, &addressx, &addressy) == 0)) {
		return -1;
	}
	if ((addressx < org->childx) || (addressx >= org->childx+org->child_width) || 
	(addressy < org->childy) || (addressy >= org->childy+org->child_height)) {
		return -1;
	}
	int c = x*10 + y;
	if ((c < 0) || (c >= N_OF_COMMANDS)) {
		return -1;
	}
	if (!(array_set_command(memory, addressx, addressy, c) == 0)) {
		return -1;
	}
	return 0;
}

int load_instance(array* memory, organism* org) {
	// load a vector corresponding to instance in one register into another
	int reg_dest = array_get_command_code(memory, org->ptrx + org->startx+org->deltax*2, org->ptry + org->starty+org->deltay*2);
	int reg_source = array_get_command_code(memory, org->ptrx + org->startx+org->deltax, org->ptry + org->starty+org->deltay);
	if ((reg_source < 0) || (reg_source >= N_OF_COMMANDS)) {
		return -1;
	}
	if ((reg_dest < 0) || (reg_dest >= N_OF_COMMANDS)) {
		return -1;
	}
	int x;
	int y;
	if (!(get_register_value(org, reg_source, &x, &y) == 0)) {
		return -1;
	}
	int command = array_get_command_code(memory, x, y);
	if ((command < 0) || (command >= N_OF_COMMANDS)) {
		return -1;
	}
	if (!(set_register_value(org, reg_dest, command/10, command%10) == 0)) {
		return -1;
	}
	return 0;
}

int TEMPLATE[1024]; //limited size
int MAX_TEMPLATE_SIZE = 1024;
int find_template(array* memory, organism* org) {
	// address by pattern
	/*
	direction: >
	|-command symbol
	||-register to store address
	|| ____ template = ~ (.:..:..) = :.::.::                  (x, y) to store
	||/    \                                                  |
	&a.:..:.._some_symbols_that_aren't_._or_:_until_hopefully_:.::.::
	*/
	int x = org->ptrx + org->startx + org->deltax;
	int y = org->ptry + org->starty + org->deltay;
	int reg = array_get_command_code(memory, x, y);

	int maxlen = (org->width < org->height) ? (org->height) : (org->width);

	x += org->deltax;
	y += org->deltay;
	int c = array_get_command_code(memory, x, y);
	int i = 0;
	while ((i < maxlen) && ((commands[c] == '.') || (commands[c] ==':' )) && (i < MAX_TEMPLATE_SIZE)) {
		TEMPLATE[i] = (commands[c] == '.') ? command_symbol_to_code(':') : command_symbol_to_code('.');
		x += org->deltax;
		y += org->deltay;
		c = array_get_command_code(memory, x, y);
		i++;
	}
	if (i == 0) {
		return -1;
	}

	int j = 0;
	while ((j < i) &&
	(org->startx <= x) && (x < org->startx+maxlen) && 
	(org->starty <= y) && (y < org->starty+maxlen)) {
		x += org->deltax;
		y += org->deltay;
		c = array_get_command_code(memory, x, y);
		if (c == TEMPLATE[j]) {
			j++;
		}
		else {
			j = 0;
		}
	}
	if (j != i) {
		return -1;
	}
	if (!(set_register_value(org, reg, x, y) == 0)) {
		return -1;
	}
	return 0;
}

int allocate_child(array* memory, organism* org) {
	// find a chunk of free memory and reserve it for copying yourself
	int child_size = array_get_command_code(memory, org->ptrx + org->startx+org->deltax, org->ptry + org->starty+org->deltay);
	int child_start = array_get_command_code(memory, org->ptrx + org->startx+org->deltax*2, org->ptry + org->starty+org->deltay*2);
	if ((child_size < 0) || (child_size >= N_OF_COMMANDS) || (child_start < 0) || (child_start >= N_OF_COMMANDS)) {
		return -1;
	}
	int width, height;
	if (!(get_register_value(org, child_size, &width, &height) == 0)) {
		return -1;
	}
	if ((width < 0) || (height < 0)) {
		return -1;
	}
	if ((width == 0) || (height == 0)) {
		return 0;
	}
	int x = org->ptrx + org->startx+org->deltax*2;
	int y = org->ptry + org->starty+org->deltay*2;
	int is_space_found = 0;
    for (int i = 2; ((i < memory->size) && (!is_space_found) && 
	(x >= 0) && (y >= 0) && (x < memory->size) && (y < memory->size)); i++) {
		x += org->deltax;
		y += org->deltay;
        if (memory_is_free(memory, x, y, width, height) == 1) {
            org->childx = x;
            org->childy = y;
			if (!(set_register_value(org, child_start, x, y) == 0)) {
				return -1;
			}
            is_space_found = 1;
		}
	}
    if (is_space_found) {
        if (!(memory_allocate(memory, org->childx, org->childy, width, height) == 0)) {
			return -1;
		}
		org->child_width = width;
		org->child_height = height;
	}
	return 0;
}

int zero(array* memory, organism* org) {
	//set value of both components of given register to 0
	int reg = array_get_command_code(memory, org->ptrx + org->startx+org->deltax, org->ptry + org->starty+org->deltay);
	if ((reg < 0) || (reg >= N_OF_COMMANDS)) {
		return -1;
	}
	if (!(set_register_value(org, reg, 0, 0) == 0)) {
		return -1;
	}
	return 0;
}

int one(array* memory, organism* org) {
	//set value of both components of given register to 1
	int reg = array_get_command_code(memory, org->ptrx + org->startx+org->deltax, org->ptry + org->starty+org->deltay);
	if ((reg < 0) || (reg >= N_OF_COMMANDS)) {
		return -1;
	}
	if (!(set_register_value(org, reg, 1, 1) == 0)) {
		return -1;
	}
	return 0;
}

int subtract(array* memory, organism* org) {
	//register_3 = register1 - register2
	int reg1 = array_get_command_code(memory, org->ptrx + 
	org->startx+org->deltax, org->ptry + org->starty+org->deltay);
	int reg2 = array_get_command_code(memory, org->ptrx + 
	org->startx+org->deltax*2, org->ptry + org->starty+org->deltay*2);
	int reg3 = array_get_command_code(memory, org->ptrx + 
	org->startx+org->deltax*3, org->ptry + org->starty+org->deltay*3);
	int xa, ya;
	int xb, yb;
	int xc, yc;
	if (!(get_register_value(org, reg1, &xa, &ya) == 0)) {
		return -1;
	}
	if (!(get_register_value(org, reg2, &xb, &yb) == 0)) {
		return -1;
	}
	if (!(get_register_value(org, reg3, &xc, &yc) == 0)) {
		return -1;
	}
	if (!(set_register_value(org, reg3, xa - xb, ya - yb) == 0)) {
		return -1;
	}
	return 0;
}

int split_child(array* memory, organism* org, table_of_organisms* table) {
	//split child - create a new, independent organism
	if ((org->child_width < 0) || (org->child_height < 0)) {
		return -1;
	}
	if ((org->child_width == 0) || (org->child_height == 0)) {
		return 0;
	}
	if (!(memory_activate(memory, org->childx, org->childy, org->child_width, org->child_height) == 0)) {
		return -1;
	}
	organism neworg;
	organism_create(&neworg, org->childx, org->childy, org->child_width, org->child_height, org->id, table);
	org->children++;
    org->reproduction_cycle = 0;
    org->child_width = 0;
	org->child_height = 0;
    org->childx = 0;
	org->childy = 0;
	org->reproduction_cycle = 0;
	return 0;
}

int operation(array* memory, organism* org, table_of_organisms* table, int command) {
	//perform a given command
	switch(command)
	{
		case '.': return 0;
		case ':': return 0;
		case 'a' : return 0;
		case 'b' : return 0;
		case 'c' : return 0;
		case 'd' : return 0;
		case 'x' : return 0;
		case 'y' : return 0;
		case '>' : return right(org);
		case '^': return up(org);
		case '<' : return left(org);
		case 'v' : return down(org);
		case '&' : return find_template(memory, org);
		case '?' : return jumpc(memory, org);
		case '1': return one(memory, org);
		case '0': return zero(memory, org);
		case '-' : return decrement(memory, org);
		case '+' : return increment(memory, org);
		case '~' : return subtract(memory, org);
		case 'L' : return load_instance(memory, org);
		case 'W' : return write_instance(memory, org);
		case '@': return allocate_child(memory, org);
		case '$': return split_child(memory, org, table);
		case 'S' : return Stack(memory, org);
		case 'P' : return Pop(memory, org);
	}
	return -1;
}



int life(array* memory, organism* org, table_of_organisms* table) {
	//lifecycle of an organism
	org->reproduction_cycle++; //organism dies if it doesn't reproduce
	move_organism_pointer(org);
	if ((org->ptrx < 0) || (org->ptrx >= memory->size)) {
		return -1;
	}
	if ((org->ptry < 0) || (org->ptry >= memory->size)) {
		return -1;
	}
	int oc = array_get_command_code(memory, org->ptrx + org->startx, org->ptry + org->starty);
	char command = commands[oc];
	int code = operation(memory, org, table, command);
	if (code != 0) {
		org->errors += 1;
	}
	if (org->reproduction_cycle > MAX_ITERS_WITHOUT_REPRODUCTION) {
		org->errors += MAX_N_OF_ERRORS; //organism dies if it doesn't reproduce
	}
	printf("O %d %d %d %c %d|", org->id, org->ptrx, org->ptry, command, code==0);
	return 0;
}

int write_chunck_from_file(array* memory, int startx, int starty, int width, int height, char* filename) {
	FILE* file = fopen(filename, "r");
	if(!file) {
	    return -1;
	}

	if ((startx < 0) || (startx >= memory->size-height) || (starty < 0) || (starty >= memory->size-width)) {
		return -1;
	}
	char c;
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			c = getc(file);
			array_set_command(memory, startx+i, starty+j, command_symbol_to_code(c));
		}
		c = getc(file);
	}
	memory_activate(memory, startx, starty, height, width);
	return 0;
}


int radiation(array* memory) {
	// a random change to allocated memory
	int x = rand() % memory->size;
	int y = rand() % memory->size;
	int z = rand() % N_OF_COMMANDS;
	if ((3 & memory->content[x*memory->size+y]) == 2) {
		return 0;
	}
	if (!(array_set_command(memory, x, y, z) == 0)) {
		return -1;
	}
	return 0;
}

int cycle(table_of_organisms* table, array* memory, int i) {
	for (int top = table->top-1; top > -1; top--) {
		life(memory, &(table->organisms[table->organisms_order[top]]), table);
	}
	if (i%1000 == 0) {
		radiation(memory);
	}
	update_queue(table, memory);
	printf("\n"); //we will read log line by line
}

/*run the program:
gcc fungera.c -o program
./program > log.txt
*/
int main() {
	int MAX_N_OF_ORGANISMS = 10000;
	table_of_organisms TABLE;
	table_create(&TABLE, MAX_N_OF_ORGANISMS);

	srand(time(0));
	array arr;
	array_create(&arr, 128);

	// Initialize the ancestor
    write_chunck_from_file(&arr, 1, 1, 23, 17, "initial.gen");

    organism cur_o;
    organism_create(&cur_o, 1, 1, 17, 23, 0, &TABLE);

	for (int i = 0; ((i < 1000000) && (TABLE.top != 0)); i++) {
		cycle(&TABLE, &arr, i);
		//if (i % 1000 == 0) {
		//	print_array(&arr);
		//print_array_mask(&arr);
		//}
    }


	free(arr.content);
	free(TABLE.organisms);
	return 0;
}
