#include <stdlib.h> 
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <ncurses.h>

#define STATUS_NCOLS 32
#define CTRL_NROWS 4
#define NCELLS 512
#define NPROPERTIES 17
#define NCOMPLEX_PROPERTIES 7

bool do_overlap(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4);
void my_mvwaddstr(WINDOW *win, int y, int x, char *s);

int MAX_N_OF_ERRORS = 5000;
int MAX_ITERS_WITHOUT_REPRODUCTION = 100000;
int TEMPLATE[1024];
int STACK_SIZE = 8;

int COMMAND_LENGTH = 5;

typedef struct {
	int size;
	char *content;
} array;

char commands[25] = {'.', ':', 'a', 'b', 'c', 'd', 'x', 'y', '^', 'v', '>', '<', '&', '?', '1', '0', '-', '+', '~', 'L', 'W', '@', '$', 'S', 'P'};
int N_OF_COMMANDS = 25;

int id = 0;
typedef struct {
	int parent_id;
	int id;
	int startx;
	int starty;
	int width;
	int height;
	int ptrx;
	int ptry;
	int deltax;
	int deltay;
	int ax;
	int ay;
	int bx;
	int by;
	int cx;
	int cy;
	int dx;
	int dy;
	int stackx[8];
	int stacky[8];
	int stacktop;
	int errors;
	// reproduction
	int reproduction_cycle;
	// child
	int childx;
	int childy;
	int child_width;
	int child_height;
	int children;
} organism;

void refresh_status_subwin(WINDOW *status_win, organism *o);

typedef struct {
	int top;
	int size;
	int* organisms_order;
	organism* organisms;
} table_of_organisms;

void refresh_mem_win(WINDOW *mem_win, int mem_nrows, int mem_ncols, array *arr, int begin_i, int begin_j, table_of_organisms *table);

int kill_organism(organism org, array* memory);

void table_create(table_of_organisms* table, int size) {
	table->top = 0;
	table->size = size;
	table->organisms_order = calloc(size, sizeof(int));
	table->organisms = calloc(size, sizeof(organism*));
}


void print_queue(table_of_organisms* table) {
	for (int j = 0; j < table->top; j++) {
//		printf("%d id %d at (%d %d) errors %d\n", j, table->organisms[j].id, table->organisms[j].startx, table->organisms[j].starty, table->organisms[j].errors);
	}
}

void update_queue(table_of_organisms* table, array* memory) {
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
	//print_queue(q);
	int end = table->top;
	for (int i = table->top-1; i >= 0; i--) {
		//printf("iteration %d organism %d (errors %d)\n", i, (q->organisms[i]).id, (q->organisms[i]).errors);
		if ((table->organisms[i].errors) > MAX_N_OF_ERRORS) {
			end = i;
			break;
		}
	}
	for (int i = end; i < table->top; i++) {
		if (!(kill_organism(table->organisms[table->organisms_order[i]], memory) == 0)) {
			return;
		}
	}
	table->top = end;
}

int append_queue(table_of_organisms* table, organism* org) {
//	printf("pushing into queue Organism %d\n", org->id);
	if (table->top == table->size) {
		return -1;
	}
	table->organisms_order[table->top] = table->top;
//	printf("appending, table top is %d (%d)\n", table->top, table->organisms_order[table->top]);
	table->organisms[table->top] = *org;
	table->top++;
	//print_queue(q);
//	printf("Q %d %d %d %d %d %d|", org->id, org->parent_id, org->startx, org->starty, org->width, org->height);
	return 0;
}

int command_symbol_to_code(char s) {
	for (int i = 0; i < N_OF_COMMANDS; i++) {
		if (s == commands[i]) {
			return i;
		}
	}
	//printf("unknown command %c\n", s);
	return -1;
}

/*
'.' : 00000001 : 'no_operation',
':' : 00000010 : 'no_operation',
'a' : 00000011 : 'no_operation',
'b' : 00000100 : 'no_operation',
'c' : 00000101 : 'no_operation',
'd' : 00000110 : 'no_operation',
'x' : 00000111 : 'no_operation',
'y' : 00001000 : 'no_operation',
'^' : 00001001 : 'move_organism_pointer_up',
'v' : 00001010 : 'move_organism_pointer_down',
'>' : 00001011 : 'move_organism_pointer_right',
'<' : 00001100 : 'move_organism_pointer_left',
'&' : 00001101 : 'find_template',
'?' : 00001110 : 'if_not_zero',
'1' : 00001111 : 'one',
'0' : 00010000 : 'zero',
'-' : 00010001 : 'decrement',
'+' : 00010010 : 'increment',
'~' : 00010011 : 'subtract',
'L' : 00010100 : 'load_inst',
'W' : 00010101 : 'write_inst',
'@' : 00010110 : 'allocate_child',
'$' : 00010111 : 'split_child',
'S' : 00011000 : 'push',
'P' : 00011001 : 'pop',
*/

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
	if (reg == 'a') {
		org->ax = x;
		org->ay = y;
	}
	else if (reg == 'b') {
		org->bx = x;
		org->by = y;
	}
	else if (reg == 'c') {
		org->cx = x;
		org->cy = y;
	}
	else if (reg == 'd') {
		org->dx = x;
		org->dy = y;
	}
	else {
		//printf("register %c doesn't exist\n", reg);
		return -1;
	}
	return 0;
}

int get_register_value(organism* org, int register_code, int* x, int* y) {
	if ((register_code < 0) || (register_code >= N_OF_COMMANDS)) {
		//printf("unknown command code %d\n", register_code);
		return -1;
	}
	char reg = commands[register_code];
	if (reg == 'a') {
		*x = org->ax;
		*y = org->ay;
	}
	else if (reg == 'b') {
		*x = org->bx;
		*y = org->by;
	}
	else if (reg == 'c') {
		*x = org->cx;
		*y = org->cy;
	}
	else if (reg == 'd') {
		*x = org->dx;
		*y = org->dy;
	}
	else {
		//printf("register %d (%c) doesn't exist\n", register_code, reg);
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
	//printf("moving organism %d by (%d %d)\n", org->id, org->deltax, org->deltay);
	org->ptrx += org->deltax;
	org->ptry += org->deltay;
}

void print_array(array* arr) {
	for (int i = 0; i < arr->size; i++) {
		for (int j = 0; j < arr->size; j++) {
//			printf("%c", commands[arr->content[i*arr->size + j] >> 2]);
		}
//		printf("\n");
	}
//	printf("\n");
}

void print_array_mask(array* arr) {
	for (int i = 0; i < arr->size; i++) {
		for (int j = 0; j < arr->size; j++) {
//			printf("%d", 3 & arr->content[i*arr->size + j]);
		}
//		printf("\n");
	}
//	printf("\n");
}


void print_field(array* arr, organism* org) {
	for (int i = 0; i < arr->size; i++) {
		for (int j = 0; j < arr->size; j++) {
			if ((org->ptrx + org->startx == i) && (org->ptry + org->starty == j)) {
				//printf("#");
			}
			else {
				//printf("%c", commands[arr->content[i*arr->size + j] >> 2]);
			}
		}
		//printf("\n");
	}
	//printf("\n");
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
	return 0;
}

char array_get(array* arr, int x, int y) {
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
//	printf("MW %d %d %c|", x, y, commands[code]);
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
	int reg = array_get_command_code(memory, org->ptrx + org->startx+org->deltax, org->ptry + org->starty+org->deltay);
	if ((reg < 0) || (reg >= N_OF_COMMANDS)) {
		return -1;
	}
	int x = 1;
	int y = 1;
	int xs;
	int ys;
	if ((commands[reg] == 'x') || (commands[reg] == 'y')) {
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
	int reg = array_get_command_code(memory, org->ptrx + org->startx+org->deltax, org->ptry + org->starty+org->deltay);
	if ((reg < 0) || (reg >= N_OF_COMMANDS)) {
		return -1;
	}
	int x = 1;
	int y = 1;
	int xs;
	int ys;
	if ((commands[reg] == 'x') || (commands[reg] == 'y')) {
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
	int reg1 = array_get_command_code(memory, org->ptrx + org->startx+org->deltax, org->ptry + org->starty+org->deltay);
	int reg2 = array_get_command_code(memory, org->ptrx + org->startx+org->deltax*2, org->ptry + org->starty+org->deltay*2);
	int addressx;
	int addressy;
	int x;
	int y;
	if (!(get_register_value(org, reg2, &x, &y) == 0)) {
		return -1;
	}
	if (!(get_register_value(org, reg1, &addressx, &addressy) == 0)) {
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

int find_template(array* memory, organism* org) {
	int x = org->ptrx + org->startx + org->deltax;
	int y = org->ptry + org->starty + org->deltay;
	int reg = array_get_command_code(memory, x, y);

	int maxlen = (org->width < org->height) ? (org->height) : (org->width);

	x += org->deltax;
	y += org->deltay;
	int c = array_get_command_code(memory, x, y);
	int i = 0;
	while ((i < maxlen) && ((commands[c] == '.') || (commands[c] ==':' ))) {
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
	org->reproduction_cycle++;
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
		org->errors += MAX_N_OF_ERRORS;
	}
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
	int x = rand()%memory->size;
	int y = rand()%memory->size;
	int z = rand()%N_OF_COMMANDS;
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
}

int main() {
	int MAX_N_OF_ORGANISMS = 50000;
	table_of_organisms TABLE;
	table_create(&TABLE, MAX_N_OF_ORGANISMS);

	srand(time(0));
	array arr;
	array_create(&arr, NCELLS);

	// Initialize the ancestor
    write_chunck_from_file(&arr, 1, 1, 23, 17, "initial.gen");

    organism org;
    organism_create(&org, 1, 1, 17, 23, 0, &TABLE);

	int n;

	organism *cur_o;
	int j = 0;
    int c;
	int begin_i = 0, begin_j = 0;
	int max_y, max_x;
    int mem_nrows, mem_ncols;
    char *s[NPROPERTIES] = {"ID:", "Parent's ID:", "BP:", "Size:", "Vector of mov:", "IP:", "A:", "B:",
                            "C:", "D:", "Stack:", "Top of stack:", "Errors:", "Repr cycle:",
                            "Child's BP:", "Child's size:", "Nchildren:"};
    int i = 0;
    bool is_running = false;

	initscr();

	cbreak();
	noecho();
	nodelay(stdscr, true);
	start_color();

	init_pair(1, COLOR_BLACK, COLOR_RED);
	init_pair(2, COLOR_BLACK, COLOR_MAGENTA);

	getmaxyx(stdscr, max_y, max_x);

    WINDOW *status_win = newwin(max_y-CTRL_NROWS, STATUS_NCOLS, 0, 0);
    refresh();
    box(status_win, 0, 0);
    mvwaddstr(status_win, 0, 1, " Status ");
    for (i = 0; i < NPROPERTIES; i++) {
        mvwaddstr(status_win, i+1, 2, s[i]);
    }
    wnoutrefresh(status_win);

    cur_o = &TABLE.organisms[j];
    WINDOW *status_subwin = newwin(max_y-CTRL_NROWS-2, STATUS_NCOLS-18, 1, 17);
    refresh_status_subwin(status_subwin, cur_o);

    mem_nrows = max_y - CTRL_NROWS;
    mem_ncols = max_x - STATUS_NCOLS;
	WINDOW *mem_win = newwin(mem_nrows, mem_ncols, 0, STATUS_NCOLS);
	box(mem_win, 0, 0);
	mvwaddstr(mem_win, 0, 1, " Memory ");
	refresh_mem_win(mem_win, mem_nrows, mem_ncols, &arr, begin_i, begin_j, &TABLE);

    WINDOW *ctrl_win = newwin(CTRL_NROWS, max_x, max_y-CTRL_NROWS, 0);
    box(ctrl_win, 0, 0);
    mvwaddstr(ctrl_win, 0, 1, " Controls ");
    my_mvwaddstr(ctrl_win, 1, 2, "\002W\003 Up\t\002S\003 Down\t\002A\003 Left\t\002D\003 Right");
    my_mvwaddstr(ctrl_win, 2, 2, "\002Q\003 Previous organism\t\002E\003Next organism");
    wnoutrefresh(ctrl_win);
    doupdate();

	for (i = 0; ((i < 10000000) && (TABLE.top != 0)); i++) {
	    // While there is input
        while ((c = getch()) != ERR || !is_running) {
            switch (c) {
                case 'w':
                case 'W':
                    if (begin_i >= 0) {
                        begin_i -= 1;
                    }
                    break;
                case 's':
                case 'S':
                    if (begin_i <= NCELLS-mem_nrows)
                    begin_i += 1;
                    break;
                case 'a':
                case 'A':
                    if (begin_j >= 0) {
                        begin_j -= 1;
                    }
                    break;
                case 'd':
                case 'D':
                    if (begin_j <= NCELLS-mem_ncols) {
                        begin_j += 1;
                    }
                    break;
                case 'z':
                case 'Z':
                    if (j > 0) {
                        cur_o = &TABLE.organisms[--j];
                    }
                    break;
                case 'x':
                case 'X':
                    if (j < TABLE.top-1) {
                        cur_o = &TABLE.organisms[++j];
                    }
                    break;
                case 'p':
                case 'P':
                    is_running = !is_running;
                default:
                    break;
            }
        }
        n = TABLE.top;
        cycle(&TABLE, &arr, i);

        refresh_status_subwin(status_subwin, cur_o);
        refresh_mem_win(mem_win, mem_nrows, mem_ncols, &arr, begin_i, begin_j, &TABLE);
        doupdate();
    }

	endwin();

	free(arr.content);
	free(TABLE.organisms);
	return 0;
}

void refresh_status_subwin(WINDOW *status_subwin, organism *o)
{
    static int ncycle = 0;
    int i;
    int complex_properties[][2] = {
            {o->starty, o->startx},
            {o->width, o->height},
            {o->deltay, o->deltax},
            {o->ay, o->ax},
            {o->by, o->bx},
            {o->cy, o->cx},
            {o->dy, o->dx},
    };

    werase(status_subwin);
    mvwprintw(status_subwin, 0, 0, "%d", o->id);
    mvwprintw(status_subwin, 1, 0, "%d", o->parent_id);
    for (i = 2; i < NCOMPLEX_PROPERTIES; i++) {
        mvwprintw(status_subwin, i, 0, "%d, %d", complex_properties[i][0], complex_properties[i][1]);
    }

    ncycle++;
    wnoutrefresh(status_subwin);
}

void refresh_mem_win(WINDOW *mem_win, int mem_nrows, int mem_ncols, array *arr, int begin_i, int begin_j, table_of_organisms *table)
{
    int k, i, j;
    int y, x;

    for (y = 1; y < mem_nrows-1; y++) {
        x = 1;
        wmove(mem_win, y, x);
        for ( ; x < mem_ncols-1; x++) {
            waddch(mem_win, commands[array_get_command_code(arr, begin_i+y, begin_j+x)]);
        }
    }
//    for (k = 0; k < table->top; k++) {
//        if (do_overlap(begin_i, begin_j, begin_i+mem_nrows, begin_j+mem_ncols, table->organisms[k].startx, table->organisms[k].starty, table->organisms[k].startx+table->organisms[k].height, table->organisms[k].starty+table->organisms[k].width)) {
//            mvaddstr(19, 2, "Overlap");
//            wattron(mem_win, COLOR_PAIR(1));
//            for (i = table->organisms[k].startx; i < table->organisms[k].width+table->organisms[k].startx; i++) {
//                j = table->organisms[k].starty;
//                wmove(mem_win, i+begin_i, j+begin_j);
//                for ( ; j < table->organisms[k].height; j++) {
//                    waddch(mem_win, commands[array_get_command_code(arr, i-begin_i, j-begin_j)]);
//                }
//            }
//            wattroff(mem_win, COLOR_PAIR(1));
//        }
//    }
    for (k = 0; k < table->top; k++) {
        y = table->organisms[k].startx - begin_i;
        x = table->organisms[k].starty - begin_j;
        wattron(mem_win, COLOR_PAIR(2));
        for (i = 0; i < table->organisms[k].width; i++) {
            for (j = 0; j < table->organisms[k].height; j++) {
                if (y+i >= 1 && x+j >= 1 && y+i < mem_nrows, x+j < mem_ncols) {
                    mvwaddch(mem_win, y + i, x + j, commands[array_get_command_code(arr, y + i, x + j)]);
                }
            }
        }
        wattroff(mem_win, COLOR_PAIR(2));
        wattron(mem_win, COLOR_PAIR(1));
        mvwaddch(mem_win, y+table->organisms[k].ptrx, x+table->organisms[k].ptry, commands[array_get_command_code(arr, y+table->organisms[k].ptrx, x+table->organisms[k].ptry)]);
        wattroff(mem_win, COLOR_PAIR(1));
    }
    wnoutrefresh(mem_win);
}

bool do_overlap(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4)
{
    if (x1 >= x4 || x3 >= x2)
        return false;
    if (y1 <= y4 || x3 <= x2)
        return false;
    return true;
}

void my_mvwaddstr(WINDOW *win, int y, int x, char *s)
{
    wmove(win, y, x);
    while (*s) {
        switch (*s) {
            case '\002':
                wattron(win, A_STANDOUT);
                break;
            case '\003':
                wattroff(win, A_STANDOUT);
                break;
            default:
                waddch(win, *s);
                break;
        }
        s++;
    }
}
