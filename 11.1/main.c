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
			uni->w++;

			for(size_t row = 0; row < uni->h; row++){
				for(size_t c = uni->w - 1; c > col; c--){
					uni->map[row][c] = uni->map[row][c-1];
				}
			}
			col++;
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
			uni->h++;

			for(size_t r = uni->h - 1; r > row; r--){
				uni->map[r] = uni->map[r-1];
			}

			uni->map[row] = malloc(sizeof(char) * 280);
			memset(uni->map[row], '.', uni->w);

			row++;
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

static inline uint64_t shortestBetween(int64_t x1, int64_t y1, int64_t x2, int64_t y2){
	return labs(x2 - x1) + labs(y2 - y1);
}

void sumPaths(universe_t *uni){
	for(size_t from = 0; from < uni->starsSize - 1; from++){
		for(size_t star = from + 1; star < uni->starsSize; star++){
			uint32_t a = shortestBetween(
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
	expandUniverse(&uni);
	findStars(&uni);
	sumPaths(&uni);
	// printMatrix(uni.map, uni.w, uni.h);

	printf("%lu\n", uni.sumShortest);

	string_destroy(data);
	return 0;
}