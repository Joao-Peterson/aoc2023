#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <regex.h>
#include <math.h>
#include "worker.h"
#include "utils.h"
#include "string+.h"
#include "number.h"

typedef struct{
	string *springs;
	uint32_t groups[75];
	size_t groupsSize;
	uint64_t arrangements;
}spring_row_t;

typedef struct{
	spring_row_t rows[1100];
	size_t rowsSize;
	uint64_t arrangements;
}springs_t;

typedef struct{
	char *start;
	uint32_t *groups;
	int64_t pos[40*5][30];
}cache_t;

int64_t getCached(cache_t *cache, const char *start, const uint32_t *groups){
	if(cache == NULL) return -1;
	return cache->pos[start - cache->start][groups - cache->groups];
}

void setCached(cache_t *cache, const char *start, const uint32_t *groups, int64_t matches){
	if(cache == NULL) return;
	cache->pos[start - cache->start][groups - cache->groups] = matches;
}

void printMatrix(char **m, size_t w, size_t h){
	for(size_t i = 0; i < h; i++){
		printf("%.*s\n", (int)w, m[i]);
	}
}

// parse file
springs_t parseInput(const string *data){

	springs_t springs = {0};

	string_ite_t iterator = string_split(data, "\n");
	for(string *line = string_next(&iterator); line != NULL; line = string_next(&iterator)){

		spring_row_t row = {0};
		string_ite_t rowite = string_split(line, " ,");
		size_t i = 0;
		for(string *token = string_next(&rowite); token != NULL; token = string_next(&rowite)){
			if(i == 0){
				row.springs = token;
			}
			else{
				row.groups[row.groupsSize] = string_to_uint(token, 10);
				row.groupsSize++;
				string_destroy(token);
			}

			i++;
		}
		
		row.groups[row.groupsSize] = 0;
		springs.rows[springs.rowsSize] = row;
		springs.rowsSize++;
	}

	return springs;
}

#define isDefective(x) (x == '?' || x == '#')
#define isWorking(x) (x == '?' || x == '.' || x == '\0')

uint64_t matchGroupV1(char *start, uint32_t *groups, cache_t *cache){
	uint64_t matches = 0;

	int64_t matchesCached = getCached(cache, start, groups);

	// printf("Matching group %d size %u on '%s'\n", cache->groups - groups, *groups, start);

	// see cache
	if(matchesCached < 0){

		// after last group, check if consumed all
		if(*groups == '\0'){
			// printf("Matching ending group on '%s'\n", start);
			bool ended = true;
			for(size_t pos = 0; start[pos] != '\0'; pos++){
				if(start[pos] == '#') ended = false;
			}

			matches = ended;
			goto end;
		}
	
		for(size_t pos = 0; start[pos] != '\0'; pos++){
			size_t i;
			bool match = true;

				// try and see if me can match the defective springs size
				for(i = 0; i < *groups && start[pos+i] != '\0'; i++){
					if(!isDefective(start[pos+i])) match = false;
				}

				// if last character was fixed, then we are counting de group size + that character, thus, invalid 
				if(pos > 0 && start[pos - 1] == '#') goto end;
				// stopped before the size of the match
				if(i < *groups) match = false;
				// the end should be a working spring or '\0'
				if(!isWorking(start[pos + i])) match = false;

				// on match, try and seek matches ahead
				if(match){
					uint64_t matchesAhead = matchGroupV1(start + pos + *groups + 1, groups + 1, cache);
					matches += matchesAhead;
				}
		}
	}
	else{
		// printf("using cached!\n");
		matches += matchesCached;
	}

	end:
	setCached(cache, start, groups, matches);
	return matches;
}

void arranges(springs_t *springs, bool useCache){
	for(uint32_t i = 0; i < springs->rowsSize; i++){
		// printf("Row %u\n", i);
		cache_t cache = {0};
		memset(cache.pos, -1, sizeof(int64_t) * 40*5 * 30);
		cache.start = springs->rows[i].springs->raw;
		cache.groups = springs->rows[i].groups;
		uint64_t matches = matchGroupV1(springs->rows[i].springs->raw, springs->rows[i].groups, useCache ? &cache : NULL);
		springs->rows[i].arrangements = matches;
		springs->arrangements += springs->rows[i].arrangements;

		printf("Row %u - '%s' - ", i, springs->rows[i].springs->raw);
		for(size_t g = 0; g < springs->rows[i].groupsSize; g++){
			if(g > 0)
				printf(",");

			printf("%u", springs->rows[i].groups[g]);
		}

		printf(". matches: %u. total: %lu\n", matches, springs->arrangements);
	}
}

springs_t *unfold(const springs_t *springs, size_t amount){
	springs_t *unfoldedSprings = malloc(sizeof(springs_t));

	unfoldedSprings->rowsSize = springs->rowsSize;
	unfoldedSprings->arrangements = 0;
	
	for(size_t r = 0; r < springs->rowsSize; r++){
		string *unfolded = string_new_sized(springs->rows[r].springs->len * amount + amount + 1);
		for(size_t c = 0; c < amount; c++){
			if(c > 0)
				string_cat_raw(unfolded, "?");
			string_cat(unfolded, springs->rows[r].springs);

			// values
			for(size_t g = 0; g < springs->rows[r].groupsSize; g++){
				unfoldedSprings->rows[r].groups[c*springs->rows[r].groupsSize + g] = springs->rows[r].groups[g];
			}
		}

		unfoldedSprings->rows[r].springs = unfolded;
		unfoldedSprings->rows[r].groupsSize = springs->rows[r].groupsSize * amount; 
		unfoldedSprings->rows[r].arrangements = 0; 
		unfoldedSprings->rows[r].groups[unfoldedSprings->rows[r].groupsSize] = 0; 
	}
	
	return unfoldedSprings;
}

// void arrangesUnfold(springs_t *springs, size_t amount){
// 	springs_t *unfolded = unfold(springs, 2);
// 	arranges(springs);
// 	arranges(unfolded);

// 	springs->arrangements = 0;
// 	for(size_t i = 0; i < springs->rowsSize; i++){
// 		uint32_t ratio = unfolded->rows[i].arrangements / springs->rows[i].arrangements;
// 		springs->rows[i].arrangements *= pow(ratio, amount - 1);
// 		springs->arrangements += springs->rows[i].arrangements;
// 	}

// 	free(unfolded);
// }

void printSprings(const springs_t *springs){
	for(size_t i = 0; i < springs->rowsSize; i++){
		printf("%s ", springs->rows[i].springs->raw);
		for(size_t j = 0; j < springs->rows[i].groupsSize; j++){
			if(j > 0)
				printf(",");

			printf("%u", springs->rows[i].groups[j]);
		}

		printf(" - Arrangements: %lu\n", springs->rows[i].arrangements);
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

	springs_t springs = parseInput(data);
	// arranges(&springs);
	// printSprings(&springs);
	// printf("Part 1: %lu\n", springs.arrangements);

 	int unfoldAmount = argv[2] == NULL ? 5 : atoi(argv[2]);
 	bool useCache = argv[3] == NULL ? true : atoi(argv[3]);
	springs_t *unfolded = unfold(&springs, unfoldAmount);
	arranges(unfolded, useCache);
	// printSprings(unfolded);
	printf("total: %lu\n", unfolded->arrangements);
	free(unfolded);

	// arrangesUnfold(&springs, 1);
	// printSprings(&springs);
	// printf("Part 2: %lu\n", springs.arrangements);
	// too low 11916500212732
	// too low    97642331839
	// 			  97642331839

	string_destroy(data);
	return 0;
}