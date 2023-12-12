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
	char **map;
	size_t w;
	size_t h;
	uint32_t starsSize;
	uint32_t starsX[500];
	uint32_t starsY[500];
	uint64_t expansion;
	uint64_t sumShortest;
}universe_t;

void printMatrix(char **m, size_t w, size_t h){
	for(size_t i = 0; i < h; i++){
		printf("%.*s\n", (int)w, m[i]);
	}
}

// parse file
universe_t parseInput(const string *data){

	universe_t uni = {0};
	char **map = malloc(sizeof(char*) * 280);

	size_t i = 0;
	string_ite_t iterator = string_split(data, "\n");

	for(string *line = string_next(&iterator); line != NULL; line = string_next(&iterator)){
		if(i==0) uni.w = line->len;
		
		map[i] = string_unwrap(line);
		map[i] = realloc(map[i], 280);

		uni.h++;
		i++;
	}

	uni.map = map;
	return uni;
}

void expandUniverse(universe_t *uni){
	// cols
	for(size_t col = 0; col < uni->w; col++){
		bool empty = true;
		for(size_t row = 0; row < uni->h; row++){
			if(uni->map[row][col] == '#'){
				empty = false;
				break;
			}
		}

		if(empty){
			for(size_t r = 0; r < uni->h; r++){
				uni->map[r][col] = '*';
			}
		}
	}

	// rows
	for(size_t row = 0; row < uni->h; row++){
		bool empty = true;
		for(size_t col = 0; col < uni->w; col++){
			if(uni->map[row][col] == '#'){
				empty = false;
				break;
			}
		}

		if(empty){
			for(size_t c = 0; c < uni->w; c++){
				uni->map[row][c] = '*';
			}
		}
	}
}

void findStars(universe_t *uni){
	for(size_t i = 0; i < uni->h; i++){
		for(size_t j = 0; j < uni->w; j++){
			if(uni->map[i][j] == '#'){
				uni->starsX[uni->starsSize] = j;
				uni->starsY[uni->starsSize] = i;
				uni->starsSize++;
			}
		}
	}
}

uint64_t shortestBetween(universe_t *uni, int64_t x1, int64_t y1, int64_t x2, int64_t y2){
	uint64_t len = 0;

	uint32_t xfrom; 
	uint32_t yfrom;
	uint32_t xto;
	uint32_t yto;

	if(x1 > x2){
		xfrom = x2;
		xto = x1;
	}
	else{
		xfrom = x1;
		xto = x2;
	}

	if(y1 > y2){
		yfrom = y2;
		yto = y1;
	}
	else{
		yfrom = y1;
		yto = y2;
	}

	for(size_t col = xfrom; col < xto; col++){
		len += (uni->map[yfrom][col] == '*' ? uni->expansion : 1);
	}
	
	for(size_t row = yfrom; row < yto; row++){
		len += (uni->map[row][xto] == '*' ? uni->expansion : 1);
	}
	
	return len;
}

void sumPaths(universe_t *uni){
	for(size_t from = 0; from < uni->starsSize - 1; from++){
		for(size_t star = from + 1; star < uni->starsSize; star++){
			uint32_t a = shortestBetween(
				uni,
				uni->starsX[from],
				uni->starsY[from],
				uni->starsX[star],
				uni->starsY[star]
			);

			printf("%lu - %lu: %u\n", from, star, a);
			uni->sumShortest += a;
		}
	}
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

	universe_t uni = parseInput(data);
	uni.expansion = 1000000;
	expandUniverse(&uni);
	findStars(&uni);
	sumPaths(&uni);
	// printMatrix(uni.map, uni.w, uni.h);

	printf("%lu\n", uni.sumShortest);

	string_destroy(data);
	return 0;
}