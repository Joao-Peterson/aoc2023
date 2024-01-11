#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <regex.h>
#include <math.h>
#include "src/worker.h"
#include "src/string+.h"
#include "src/number.h"
#include "src/hash.h"
#include "src/data.h"
#include "src/flood_fill.h"

typedef struct{
	size_t x;
	size_t y;
}point_t;

typedef struct{
	matrix_t *map;
	point_t start; 
}puzzle_t;

// parse file
puzzle_t *parseInput(const string *data){
	puzzle_t *p = malloc(sizeof(puzzle_t));
	p->map = matrix_from_string(data);

	for(size_t i = 0; i < p->map->h; i++){
		for(size_t j = 0; j < p->map->w; j++){
			if(p->map->rows[i][j] == 'S'){
				p->start.x = j;
				p->start.y = i;
			}
		}
	}
	
	return p;
}

uint64_t part1(puzzle_t *p){
	matrix_t *copy = matrix_copy(p->map);
	flood_fill_matrix_distance(copy, p->start.x, p->start.y, '#', '.', 6);
	
	string *s = matrix_print_trunc(copy, 3);
	// string *s = matrix_print_char(p->map, 0, '\n');
	string_println(s);

	string_destroy(s);
	matrix_destroy(copy);
	return 0;
}

uint64_t part2(puzzle_t *p){
	return 0;
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

	puzzle_t *p = parseInput(data);
	
	printf("Part 1: %lu\n", part1(p));
	printf("Part 2: %lu\n", part2(p));

	string_destroy(data);
	return 0;
}