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

typedef struct map_t map_t;

struct map_t{
	int w;
	int h;
	char *terrain[30];
	uint64_t symmetry;
	uint64_t symmetryS;
	int vSym;
	int vSymS;
	int hSym;
	int hSymS;
	map_t *next;
};

int64_t bitlog2(uint64_t value){
	uint32_t pow = 0; 
	int64_t found = -1;

	do{
		if(value & 1){
			if(found == -1)
				found = pow;
			else
				return -1;
		}
		
		pow++;
	}while(value >>= 1);
	
	return found;
}

void symmetryVertical(map_t *map){
	int64_t bufferCol[30];
	int64_t breaks[30] = {0};
	int breaksQty = 0;
	for(int i = 0; i < map->w; i++){
		int64_t acc = 0;
		for(int j = 0; j < map->h; j++){
			if(map->terrain[j][i] == '#'){
				acc |= (1<<j); // i miss doing magic :')
			}
		}

		bufferCol[i] = acc;	

		if(i > 0 && (bufferCol[i] == bufferCol[i-1] || bitlog2(labs(bufferCol[i] ^ bufferCol[i-1])) >= 0)){
			breaks[breaksQty] = i; // save two equals 
			breaksQty++;
		}
	}
	
	if(breaksQty){
		for(int k = 0; k < breaksQty; k++){
			int brak = breaks[k];
			bool valid = true;
			bool validSmudge = true;
			bool unSmudge = false;
			// check if full reflection
			for(int i = 0; i < brak; i++){
				int l = brak -1 - i;
				int r = brak + i;
				if(
					r < map->w &&
					bufferCol[l] != bufferCol[r]
				){
					valid = false;

					// unsmudge. The difference of the bits is a single bit
					if(!unSmudge && bitlog2(labs(bufferCol[l] ^ bufferCol[r])) >= 0){
						unSmudge = true;	
					}
					else{
						validSmudge = false;
					}
				}	
			}

			if(valid){
				map->vSym = brak;
			}
			else if(validSmudge){
				map->vSymS = brak;
			}
		}
	}
}

void symmetryHorizontal(map_t *map){
	int64_t bufferRow[30];
	int64_t breaks[30] = {0};
	int breaksQty = 0;
	for(int i = 0; i < map->h; i++){
		int64_t acc = 0;
		for(int j = 0; j < map->w; j++){
			if(map->terrain[i][j] == '#'){
				acc |= (1<<j); // i miss doing magic :')
			}
		}

		bufferRow[i] = acc;	

		if(i > 0 && (bufferRow[i] == bufferRow[i-1] || bitlog2(labs(bufferRow[i] ^ bufferRow[i-1])) >= 0)){
			breaks[breaksQty] = i; // save two equals 
			breaksQty++;
		}
	}
	
	if(breaksQty){
		for(int k = 0; k < breaksQty; k++){
			int brak = breaks[k];
			bool valid = true;
			bool validSmudge = true;
			bool unSmudge = false;
			// check if full reflection
			for(int i = 0; i < brak; i++){
				int t = brak - 1 - i;
				int b = brak + i;
				if(
					b < map->h &&
					bufferRow[t] != bufferRow[b]
				){
					// 0 if not full
					valid = false;

					// unsmudge. The difference of the bits is a single bit
					if(!unSmudge && bitlog2(labs(bufferRow[t] ^ bufferRow[b])) >= 0){
						unSmudge = true;	
					}
					else{
						validSmudge = false;
					}
				}	
			}

			if(valid){
				map->hSym = brak;
			}
			else if(validSmudge){
				map->hSymS = brak;
			}
		}
	}
}

void symmetry(map_t *map){
	map->symmetry = 0;
	symmetryVertical(map);
	symmetryHorizontal(map);

	map->symmetry += map->vSym;
	map->symmetryS += map->vSymS;
	map->symmetry += map->hSym * 100ull;
	map->symmetryS += map->hSymS * 100ull;
}

void printMatrix(char **m, size_t w, size_t h){
	for(size_t i = 0; i < h; i++){
		printf("%.*s\n", (int)w, m[i]);
	}
}

uint64_t symmetries(map_t *maps){
	uint64_t acc = 0;
	for(map_t *cursor = maps; cursor != NULL; cursor = cursor->next){
		symmetry(cursor);
		// printf("Symmetry: %lu\n", cursor->symmetry);
		// printMatrix(cursor->terrain, cursor->w, cursor->h);
		// printf("\n");

		acc += cursor->symmetry;
	}
	return acc;
}

uint64_t symmetriesS(map_t *maps){
	uint64_t acc = 0;
	int i = 0;
	for(map_t *cursor = maps; cursor != NULL; cursor = cursor->next){
		// symmetry(cursor);
		printf("Symmetry Smudge [%i]: %lu\n", i, cursor->symmetryS);
		printMatrix(cursor->terrain, cursor->w, cursor->h);
		printf("\n");

		i++;
		acc += cursor->symmetryS;
	}
	return acc;
}

// parse file
void *parseInput(const string *data){

	string_ite_t iterator = string_split(data, "\n");
	map_t *base = calloc(1, sizeof(map_t));
	map_t *map = base;
	for(string *line = string_next(&iterator); line != NULL; line = string_next(&iterator)){
		if(line->raw[0] != ' '){
			if(map->h == 0)
				map->w = line->len;

			map->terrain[map->h] = string_unwrap(line);
			map->h++;
		}
		else{
			map->next = calloc(1, sizeof(map_t));
			map = map->next;
		}
	}

	return base;
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

	map_t *maps = parseInput(data);

	// too low 19859
	// to high 33324
	printf("Part 1: %lu\n", symmetries(maps));

	// to high 28916
	// to high 47260
	// to high 99738
	printf("Part 2: %lu\n", symmetriesS(maps));

	string_destroy(data);
	return 0;
}