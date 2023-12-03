#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <regex.h>
#include "utils.h"

typedef struct gear_t gear_t;
struct gear_t{
	int x, y;
	int z1, z2;
	gear_t *next;
};	

gear_t *addGear(gear_t *root, int x, int y, int z){
	if(root == NULL){
		gear_t *new = calloc(sizeof(gear_t), 1);
		new->x = x;
		new->y = y;
		new->z1 = z;
		new->z2 = 0;
		new->next = NULL;
		return new;
	} 
	
	gear_t *cursor = root;

	// run
	while(cursor->next != NULL){
		// if same gear, add
		if(cursor->x == x && cursor->y == y){
			if(cursor->z2 == 0) cursor->z2 = z;
			return root;
		}
		cursor = cursor->next;
	}
		
	// check last
	if(cursor->x == x && cursor->y == y){
		if(cursor->z2 == 0) cursor->z2 = z;
		return root;
	}
	
	// if not found, insert
	gear_t *new = calloc(sizeof(gear_t), 1);
	new->x = x;
	new->y = y;
	new->z1 = z;
	new->z2 = 0;
	new->next = NULL;

	cursor->next = new;

	return root;
}

uint64_t gearsAnalyse(gear_t *gears){
	if(gears == NULL) return 0;
	
	printf("calc gear %d and %d\n", gears->z1, gears->z2);
	if(gears->next == NULL){
		uint64_t ratio = gears->z1 * gears->z2;
		free(gears);
		return ratio;
	}
	else{
		uint64_t ratio = gears->z1 * gears->z2 + gearsAnalyse(gears->next);
		free(gears);
		return ratio;
	}
}

bool isSymbol(char c){
	return !(isdigit(c) || c == '.');
}

typedef struct{
	char sym;
	int y;
	int x;
}symbol_t;

symbol_t checkPart(char **sch, int rows, int cols, int *row, int *col, int len){
	int x = *col - 1;
	int y;

	// check left side
	if(x >= 0 && x < cols){
		// tl
		y = *row - 1;
		if(y >= 0 && y < rows && isSymbol(sch[y][x])) return (symbol_t){.y = y, .x = x, .sym = sch[y][x]};

		// l
		y = *row;
		if(y >= 0 && y < rows && isSymbol(sch[y][x])) return (symbol_t){.y = y, .x = x, .sym = sch[y][x]};

		// bl
		y = *row + 1;
		if(y >= 0 && y < rows && isSymbol(sch[y][x])) return (symbol_t){.y = y, .x = x, .sym = sch[y][x]};
	}

	// up and down
	y = *row;
	for(x = *col; (x < (*col + len)) && (x < cols); x++){
		if(y - 1 >= 0 	&& isSymbol(sch[y - 1][x])) return (symbol_t){.y = y - 1, .x = x, .sym = sch[y - 1][x]};
		if(y + 1 < rows && isSymbol(sch[y + 1][x])) return (symbol_t){.y = y + 1, .x = x, .sym = sch[y + 1][x]};
	}

	// check right side 
	x = *col + len;
	if(x >= 0 && x < cols){
		// tr
		y = *row - 1;
		if(y >= 0 && y < rows && isSymbol(sch[y][x])) return (symbol_t){.y = y, .x = x, .sym = sch[y][x]};

		// r
		y = *row;
		if(y >= 0 && y < rows && isSymbol(sch[y][x])) return (symbol_t){.y = y, .x = x, .sym = sch[y][x]};

		// br
		y = *row + 1;
		if(y >= 0 && y < rows && isSymbol(sch[y][x])) return (symbol_t){.y = y, .x = x, .sym = sch[y][x]};
	}

	return (symbol_t){.y = 0, .x = 0, .sym = 0};
}

uint64_t getPartNumber(char **sch, int rows, int cols, int *row, int *col, gear_t **gears){
	// get num
	char buffer[50] = {0};
	int start = *col;

	while(isdigit(sch[*row][*col])){
		(*col)++;
	}

	memcpy(buffer, sch[*row] + start, *col - start);
	int num = atoi(buffer);
	symbol_t check = checkPart(sch, rows, cols, row, &start, *col - start);

	if(check.sym == '*') 
		*gears = addGear(*gears, check.x, check.y, num);

	if(check.sym != 0){
		// printf("number: %d. IS a part!\n", num);
		return num;
	}
	else{
		// printf("number: %d. NOT a part!\n", num);	
		return 0;
	} 
}

uint64_t scanSch(char **sch, int rows, int cols){
	uint64_t acc = 0;
	int row = 0;
	int col = 0;
	gear_t *gears = NULL;

	while(row < rows){
		while(col < cols){
			if(isdigit(sch[row][col])){
				uint64_t partNum = getPartNumber(sch, rows, cols, &row, &col, &gears);
				acc += partNum;
			}

			col++;
		}

		col = 0;
		row++;
	}
	
	return gearsAnalyse(gears);
}

int main(int argc, char**argv){
	char *data;
	
	// from arg filename
	if(argc > 1){
		data = filetomem(argv[1], NULL);
	}
	// from pipe
	else{
		data = malloc(1024*1000);
		char buffer[1024];
		*data = '\0';
		*buffer = '\0';
		while(fgets(buffer, 1024, stdin) != NULL){
			strcat(data, buffer);
		}
	}

	// columns
	size_t cols = 0;
	// lines/rows
	size_t rows = 0;
	for(char *cursor = data; *cursor != '\0'; cursor++){
		if(*cursor == '\n'){
			if(cols == 0) cols = cursor - data;
			rows++;
		}
	}
	rows++;

	// create matrix with thousand lines
	char** sch = calloc(sizeof(char*), rows);
	for(size_t i = 0; i < rows; i++)
		sch[i] = data + (cols + 1) * i;

	printf("%lu\n", scanSch(sch, 140, 140));

	free(sch);
	free(data);
	return 0;
}