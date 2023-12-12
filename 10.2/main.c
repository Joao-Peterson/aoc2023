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

typedef struct node_t node_t;

struct node_t{
	size_t x;
	size_t y;
	node_t *prev;
	node_t *next;
};

typedef struct{
	// queue
	size_t size;
	node_t *first;
	node_t *last;
	
	size_t height;
	size_t width;
	char** matrix;
}fill_t;

void fill_addNode(fill_t *fill, size_t x, size_t y){
	node_t *n = malloc(sizeof(node_t));
	n->x = x; 
	n->y = y; 
	if(fill->size == 0){
		n->next = NULL;
		n->prev = NULL;
		fill->first = n;	
		fill->last = n;	
		fill->size = 1;
	}
	else{
		node_t *last = fill->last;
		n->next = last;
		n->prev = NULL;
		last->prev = n;

		fill->last = n;
		fill->size++;
	}
}

node_t *fill_getNode(fill_t *fill){
	if(fill->size == 0) return NULL;

	if(fill->size == 1){
		node_t *first = fill->first;
		fill->first = NULL;
		fill->last = NULL;
		fill->size = 0;
		return first;
	}
	else{
		node_t *first = fill->first;
		first->prev->next = NULL;
		fill->first = first->prev;
		fill->size--;
		return first;
	}
}

fill_t *fill_new(char** matrix, size_t height, size_t width){
	fill_t *fill = malloc(sizeof(fill_t));

	fill->matrix = matrix;
	fill->height = height;
	fill->width = width;
	fill->first = NULL;
	fill->last = NULL;
	fill->size = 0;

	return fill;
}

void fill_fill(fill_t *fill, size_t startx, size_t starty, char empty, char fillChar){
	fill_addNode(fill, startx, starty);

	while(fill->size != 0){
		node_t *n = fill_getNode(fill);

		if(fill->matrix[n->y][n->x] == empty){
			fill->matrix[n->y][n->x] = fillChar;

			// left
			if(n->x > 0)
				fill_addNode(fill, n->x - 1, n->y);

			// right
			if(n->x < (fill->width - 1))
				fill_addNode(fill, n->x + 1, n->y);

			// up
			if(n->y > 0)
				fill_addNode(fill, n->x, n->y - 1);

			// down
			if(n->y < (fill->height - 1))
				fill_addNode(fill, n->x, n->y + 1);
		}

		free(n);
	}
}

typedef struct{
	size_t sx, sy;
	char **map;
	char **area;
	uint32_t max;
	uint32_t insideArea;
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

void paint(pipes_t *pipes, uint32_t x, uint32_t y, char p){
	pipes->area[y][x] = pipes->map[y][x];
	// pipes->area[y][x] = p;
}

uint32_t march(
	pipes_t *pipes,
	uint32_t acc,
	uint32_t x1, uint32_t y1, dir_t from1,
	uint32_t x2, uint32_t y2, dir_t from2
){
	paint(pipes, x1, y1, '#');	
	paint(pipes, x2, y2, '#');	
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

void run(pipes_t *pipes){
	pipes->map[62][111] = 'J';
	paint(pipes, 111, 62, '#');	
	pipes->max = march(
		pipes,
		1,
		110, 62, dir_right,
		111, 61, dir_down 
	);
}

void pipesInsideArea(pipes_t *pipes){
	pipes->insideArea = 0;
	for(size_t i = 0; i < 140; i++){
		bool up = false;
		bool down = false;
		for(size_t j = 0; j < 140; j++){
			switch(pipes->area[i][j]){
				case ' ':
					if(up || down){
						pipes->area[i][j] = 'O';
						pipes->insideArea++;
					}
				break;
				
				case '|':
					up ^= true;
					down ^= true;
				break;

				case 'F':
				case '7':
					down ^= true;
				break;

				case 'S':
				case 'J':
				case 'L':
					up ^= true;
				break;

				default:
				break;
			}
		}
	}
}

uint32_t printPipesArea(const pipes_t *pipes){
	uint32_t acc = 0;
	for(size_t i = 0; i < 140; i++){
		printf("%.*s\n", 139, pipes->area[i]);
		for(size_t j = 0; j < 140; j++){
			if(pipes->area[i][j] == ' ') 
				acc++;
		}
	}
	return acc;
}

// parse file
pipes_t parseInput(const string *data){

	pipes_t pipes = {0};
	char **map = malloc(sizeof(char*) * 140);
	char **area = malloc(sizeof(char*) * 140);

	size_t i = 0;
	string_ite_t iterator = string_split(data, "\n");

	for(string *line = string_next(&iterator); line != NULL; line = string_next(&iterator)){
		map[i] = string_unwrap(line);
		area[i] = calloc(sizeof(char), 140);
		memset(area[i], ' ', 140);

		// zero over the \n
		map[i][140] = '\0';

		// find S
		const char *x = strchr(map[i], 'S');
		if(x){
			pipes.sx = x - map[i];
			pipes.sy = i;
		}

		i++;
	}

	pipes.map = map;
	pipes.area = area;
	run(&pipes);
	pipesInsideArea(&pipes);
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
	printf("%u\n", pipes.max);
	printf("%u\n", pipes.insideArea);
	// printPipesArea(&pipes);

	// fill_t *fill = fill_new(pipes.area, 140, 140);
	// fill_fill(fill, 70, 70, ' ', '@');
	// fill_fill(fill, 0, 0, ' ', '@');
	// free(fill);
	string_destroy(data);
	return 0;
}