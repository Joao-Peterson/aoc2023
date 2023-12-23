#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <regex.h>
#include <math.h>
#include "src/worker.h"
#include "src/utils.h"
#include "src/string+.h"
#include "src/number.h"

void printMatrix(char **m, size_t w, size_t h){
	for(size_t i = 0; i < h; i++){
		printf("%.*s\n", (int)w, m[i]);
	}
}

typedef struct{
	char **mirrors;
	char **visited;
	int w;
	int h;
}machine_t;

typedef enum{
	dir_up = 1,
	dir_down,
	dir_right,
	dir_left,
	dir_cross
}dir_t;

// parse file
void *parseInput(const string *data){

	string_ite_t iterator = string_split(data, "\n");
	machine_t *machine = calloc(1, sizeof(machine_t));
	machine->mirrors = calloc(120, sizeof(char*));
	machine->visited = calloc(120, sizeof(char*));

	for(string *line = string_next(&iterator); line != NULL; line = string_next(&iterator)){
		machine->w = line->len;
		machine->mirrors[machine->h] = string_unwrap(line);
		machine->visited[machine->h] = calloc(machine->w + 1, sizeof(char));
		machine->h++;
	}

	return machine;
}

void beam(machine_t *m, int x, int y, dir_t dir){
	// if out of bounds, skip
	while(x >= 0 && x < m->w && y >= 0 && y < m->h){
		// if on top of same dir, return
		if(m->visited[y][x] == (char)dir){
			return;
		}
		
		// mark cross
		if((dir == dir_left || dir == dir_right) && (m->visited[y][x] == dir_up || m->visited[y][x] == dir_down))
			m->visited[y][x] = dir_cross;
		// mark dir
		else
			m->visited[y][x] = dir;
		
		// check dir
		switch(m->mirrors[y][x]){
			// mark on empty
			default:
			case '.':
			break;

			// mirror
			case '/':
				switch(dir){
					case dir_right: dir = dir_up;    break;
					case dir_up:    dir = dir_right; break;
					case dir_down:  dir = dir_left;  break;
					case dir_left:  dir = dir_down;  break;
					default: break;
				}
			break;

			// mirror
			case '\\':
				switch(dir){
					case dir_right:	dir = dir_down;  break;
					case dir_down: 	dir = dir_right; break;
					case dir_up:   	dir = dir_left;  break;
					case dir_left: 	dir = dir_up;    break;
					default: break;
				}
			break;

			// split
			case '-':
				switch(dir){
					case dir_down:
					case dir_up:
						beam(m, x-1, y, dir_left);
						beam(m, x+1, y, dir_right);
						return;
						
					default: break;
				}
			break;

			// split
			case '|':
				switch(dir){
					case dir_left:
					case dir_right:
						beam(m, x, y-1, dir_up);
						beam(m, x, y+1, dir_down);
						return;

					default: break;
				}
			break;
		}

		// inc
		x += (dir == dir_right ? 1 : 0);
		x += (dir == dir_left ? -1 : 0);
		y += (dir == dir_down ? 1 : 0);
		y += (dir == dir_up ? -1 : 0);
	}
}

void emptyVisited(machine_t *m){
	for(int i = 0; i < m->h; i++){
		if(m->visited[i] != NULL)
			memset(m->visited[i], 0, m->w);
	}
}

uint64_t visited(const machine_t *m){
	uint64_t acc = 0;
	for(int i = 0; i < m->h; i++){
		for(int j = 0; j < m->w; j++){
			if(m->visited[i][j])
				acc++;
		}
	}
	return acc;
}

uint64_t part1(machine_t *m){
	beam(m, 0, 0, dir_right);
	return visited(m);
}

uint64_t part2(machine_t *m){
	uint64_t max = 0;
	uint64_t v;

	// left/right
	for(int i = 0; i < m->h; i++){
		// to right
		beam(m, 0, i, dir_right);
		v = visited(m);
		if(v > max) max = v;
		emptyVisited(m);
		
		// to left
		beam(m, m->w - 1, i, dir_left);
		v = visited(m);
		if(v > max) max = v;
		emptyVisited(m);
	}

	// up/down
	for(int i = 0; i < m->w; i++){
		// to below
		beam(m, i, 0, dir_down);
		v = visited(m);
		if(v > max) max = v;
		emptyVisited(m);
		
		// to above
		beam(m, i, m->h - 1, dir_up);
		v = visited(m);
		if(v > max) max = v;
		emptyVisited(m);
	}

	return max;
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

	machine_t *machine = parseInput(data);
	
	printf("Part 1: %lu\n", part1(machine));
	printf("Part 2: %lu\n", part2(machine));

	string_destroy(data);
	return 0;
}