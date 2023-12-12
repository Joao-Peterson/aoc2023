#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <regex.h>
#include "worker.h"
#include "utils.h"
#include "string+.h"
#include "number.h"

typedef struct{
	size_t sx, sy;
	char **map;
}pipes_t;

// over 0x01000 cause ascii char are in the 0x00FF range
typedef enum{
	dir_none  = 0x0000,
	dir_left  = 0x0100,
	dir_right = 0x0200,
	dir_up    = 0x0400,
	dir_down  = 0x0800
}dir_t;

void step(char pipe, dir_t from, uint32_t *xnew, uint32_t *ynew, dir_t *fromnew){
	switch(pipe | from){
		case '|' | dir_down:
			(*ynew)--; // goes up
			*fromnew = dir_down;
		break;

		case '|' | dir_up:
			(*ynew)++; // goes down
			*fromnew = dir_up;
		break;

		case '-' | dir_left:
			(*xnew)++;
			*fromnew = dir_left;
		break;

		case '-' | dir_right:
			(*xnew)--;
			*fromnew = dir_right;
		break;
		
		case 'L' | dir_right:
			(*ynew)--;
			*fromnew = dir_down;
		break;

		case 'L' | dir_up:
			(*xnew)++;
			*fromnew = dir_left;
		break;
		
		case 'J' | dir_left:
			(*ynew)--;
			*fromnew = dir_down;
		break;
		
		case 'J' | dir_up:
			(*xnew)--;
			*fromnew = dir_right;
		break;
		
		case '7' | dir_down:
			(*xnew)--;
			*fromnew = dir_right;
		break;
		
		case '7' | dir_left:
			(*ynew)++;
			*fromnew = dir_up;
		break;
		
		case 'F' | dir_down:
			(*xnew)++;
			*fromnew = dir_left;
		break;
		
		case 'F' | dir_right:
			(*ynew)++;
			*fromnew = dir_up;
		break;
		
		default:
			printf("we fucked up! Symbol: %c, pos: %u, %u. dir: %X\n", pipe, *xnew, *ynew, from);
			exit(1);
		break;
	}
}

uint32_t march(
	const pipes_t *pipes,
	uint32_t acc,
	uint32_t x1, uint32_t y1, dir_t from1,
	uint32_t x2, uint32_t y2, dir_t from2
){
	if(x1 == x2 && y1 == y2) return acc;

	// path 1
	uint32_t x1new = x1; 
	uint32_t y1new = y1; 
	dir_t from1new = from1;
	step(pipes->map[y1][x1], from1, &x1new, &y1new, &from1new);

	// path 2
	uint32_t x2new = x2; 
	uint32_t y2new = y2; 
	dir_t from2new = from2;
	step(pipes->map[y2][x2], from2, &x2new, &y2new, &from2new);

	return march(
		pipes, 
		acc + 1, 
		x1new, y1new, from1new,
		x2new, y2new, from2new
	);
}

uint32_t run(const pipes_t *pipes){
	return march(
		pipes,
		1,
		110, 62, dir_right,
		111, 61, dir_down 
	);
}

// parse file
pipes_t parseInput(const string *data){

	pipes_t pipes = {0};
	char **map = malloc(sizeof(char*) * 140);

	size_t i = 0;
	string_ite_t iterator = string_split(data, "\n");

	for(string *line = string_next(&iterator); line != NULL; line = string_next(&iterator)){
		map[i] = string_unwrap(line);
		
		// zero over the \n
		map[i][140] = '\0';

		// find S
		char *x = strchr(map[i], 'S');
		if(x){
			pipes.sx = x - map[i];
			pipes.sy = i;
		}

		i++;
	}

	pipes.map = map;
	return pipes;
}

int main(int argc, char**argv){
	string *data;
	
	// from arg filename
	if(argc > 1){
		data = string_from_filename(argv[1], NULL);
	}
	// from pipe
	else{
		data = string_from_file(stdin, NULL);
	}

	pipes_t pipes = parseInput(data);
	printf("%u\n", run(&pipes));

	string_destroy(data);
	return 0;
}