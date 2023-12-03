#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <regex.h>
#include "utils.h"

bool isSymbol(char c){
	return !(isdigit(c) || c == '.');
}

bool checkPart(char **sch, int rows, int cols, int *row, int *col, int len){
	int x = *col - 1;
	int y;

	// check left side
	if(x >= 0 && x < cols){
		// tl
		y = *row - 1;
		if(y >= 0 && y < rows && isSymbol(sch[y][x])) return true;

		// l
		y = *row;
		if(y >= 0 && y < rows && isSymbol(sch[y][x])) return true;

		// bl
		y = *row + 1;
		if(y >= 0 && y < rows && isSymbol(sch[y][x])) return true;
	}

	// up and down
	y = *row;
	for(x = *col; (x < (*col + len)) && (x < cols); x++){
		if(y - 1 >= 0 	&& isSymbol(sch[y - 1][x])) return true;
		if(y + 1 < rows && isSymbol(sch[y + 1][x])) return true;
	}

	// check right side 
	x = *col + len;
	if(x >= 0 && x < cols){
		// tr
		y = *row - 1;
		if(y >= 0 && y < rows && isSymbol(sch[y][x])) return true;

		// r
		y = *row;
		if(y >= 0 && y < rows && isSymbol(sch[y][x])) return true;

		// br
		y = *row + 1;
		if(y >= 0 && y < rows && isSymbol(sch[y][x])) return true;
	}

	return false;
}

uint64_t getPartNumber(char **sch, int rows, int cols, int *row, int *col){
	// get num
	char buffer[50] = {0};
	int start = *col;

	while(isdigit(sch[*row][*col])){
		(*col)++;
	}

	memcpy(buffer, sch[*row] + start, *col - start);
	int num = atoi(buffer);
	if(checkPart(sch, rows, cols, row, &start, *col - start)){
		printf("number: %d. IS a part!\n", num);
		return num;
	}
	else{
		printf("number: %d. NOT a part!\n", num);	
		return 0;
	} 
}

uint64_t scanSch(char **sch, int rows, int cols){
	uint64_t acc = 0;
	int row = 0;
	int col = 0;

	while(row < rows){
		while(col < cols){
			if(isdigit(sch[row][col])){
				uint64_t partNum = getPartNumber(sch, rows, cols, &row, &col);
				acc += partNum;
			}

			col++;
		}

		col = 0;
		row++;
	}
	
	return acc;
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