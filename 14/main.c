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
#include "src/hash.h"

typedef struct{
	char *rocks[100];
	int w;
	int h;
}platform_t;

void printMatrix(char **m, size_t w, size_t h){
	for(size_t i = 0; i < h; i++){
		printf("%.*s\n", (int)w, m[i]);
	}
}

uint32_t hash_rocks(const platform_t *p){
	uint64_t acc = 0;
	for(int i = 0; i < p->h; i++){
		acc ^= djb2_hash_string(p->rocks[i]) * i;
	}

	return acc % 100000; // cap 100000
}

// parse file
void *parseInput(const string *data){

	string_ite_t iterator = string_split(data, "\n");
	platform_t *p = calloc(1, sizeof(platform_t));
	for(string *line = string_next(&iterator); line != NULL; line = string_next(&iterator)){
		p->w = line->len;
		p->rocks[p->h] = string_unwrap(line);
		p->h++;
	}

	return p;
}

uint64_t loadNorth(const platform_t *p){
	uint64_t load = 0;
	for(int h = 0; h < p->h; h++){
		for(int w = 0; w < p->w; w++){
			if(p->rocks[h][w] == 'O'){
				load += p->h - h;
			}
		}
	}
	return load;
}

int tiltNorth(platform_t *p){
	int changes = 0;
	for(int h = 1; h < p->h; h++){
		for(int w = 0; w < p->w; w++){
			if(p->rocks[h][w] == 'O' && p->rocks[h-1][w] == '.'){
				p->rocks[h][w] = '.';
				p->rocks[h-1][w] = 'O';
				changes++;
			}
		}
	}
	return changes;
}

int tiltSouth(platform_t *p){
	int changes = 0;
	for(int h = p->h - 2; h >= 0; h--){
		for(int w = 0; w < p->w; w++){
			if(p->rocks[h][w] == 'O' && p->rocks[h+1][w] == '.'){
				p->rocks[h][w] = '.';
				p->rocks[h+1][w] = 'O';
				changes++;
			}
		}
	}
	return changes;
}

int tiltEast(platform_t *p){
	int changes = 0;
	for(int w = p->w - 2; w >= 0; w--){
		for(int h = 0; h < p->h; h++){
			if(p->rocks[h][w] == 'O' && p->rocks[h][w+1] == '.'){
				p->rocks[h][w] = '.';
				p->rocks[h][w+1] = 'O';
				changes++;
			}
		}
	}
	return changes;
}

int tiltWest(platform_t *p){
	int changes = 0;
	for(int w = 1; w < p->w; w++){
		for(int h = 0; h < p->h; h++){
			if(p->rocks[h][w] == 'O' && p->rocks[h][w-1] == '.'){
				p->rocks[h][w] = '.';
				p->rocks[h][w-1] = 'O';
				changes++;
			}
		}
	}
	return changes;
}

typedef enum{
	dir_north,
	dir_west,
	dir_south,
	dir_east,
}dir_t;

int tilt(platform_t *p, dir_t dir){
	int changes = 0;
	int changesAcc = 0;
	switch(dir){
		default:
		case dir_north:
			do{
				changes = tiltNorth(p);
				changesAcc += changes;
			}while(changes > 0);
		break;

		case dir_south:
			do{
				changes = tiltSouth(p);
				changesAcc += changes;
			}while(changes > 0);
		break;

		case dir_east:
			do{
				changes = tiltEast(p);
				changesAcc += changes;
			}while(changes > 0);
		break;

		case dir_west:
			do{
				changes = tiltWest(p);
				changesAcc += changes;
			}while(changes > 0);
		break;
	}
	return changesAcc;
}

int spin(platform_t *p){
	int changes = 0;
	for(dir_t i = dir_north; i <= dir_east; i++){
		changes = tilt(p, i);
		// printMatrix(p->rocks, p->w, p->h);
		// printf("\n");
	}
	return changes;
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

	platform_t *p = parseInput(data);
	tilt(p, dir_north);
	printf("Part 1: %lu\n", loadNorth(p));

	uint32_t indexes[100000] = {0};
	uint32_t loads[100000] = {0};
	
	uint32_t first = 0;
	uint32_t i;
	uint32_t hash = hash_rocks(p);
	uint32_t off = 0;
	// printf("Hash: %u\n", hash);
	// printMatrix(p->rocks, p->w, p->h);
	// printf("\n");
	for(i = 0; i < 100000; i++){
		spin(p);
		hash = hash_rocks(p);
		// printf("Hash: %u\n", hash);

		if(indexes[hash] && off){
			off--;
		}
		else if(indexes[hash] && !off){
			first = indexes[hash];
			break;
		}

		uint32_t load = loadNorth(p);
		printf("Load %u: %u\n", i, load);
		indexes[hash] = i;
		loads[i] = load;
	}

	// uint32_t ibth = ((1000000000 - first) % (i - first)) + first;
	uint32_t ibth = 1000000000 - first;
	ibth = ibth % (i - first);
	ibth += first - 1;
	uint32_t loadbth = loads[ibth];

	// too low 118739
	printf("Part 2: %u\n", loadbth);

	string_destroy(data);
	return 0;
}