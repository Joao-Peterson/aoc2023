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

typedef struct{
	
}puzzle_t;

// parse file
puzzle_t *parseInput(const string *data){
	puzzle_t *p = malloc(sizeof(puzzle_t));

	string_ite_t iterator = string_split(data, "\n");
	size_t l = 0;
	for(string *line = string_next(&iterator); line != NULL; line = string_next(&iterator)){

		
		l++;
		string_destroy(line);
	}
	
	return p;
}

uint64_t part1(puzzle_t *p){
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