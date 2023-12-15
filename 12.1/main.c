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
	string *springs;
	uint32_t groups[20];
	size_t groupsSize;
	uint64_t arrangements;
}spring_row_t;

typedef struct{
	spring_row_t rows[1100];
	size_t rowsSize;
	uint64_t arrangements;
}springs_t;

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
		
		springs.rows[springs.rowsSize] = row;
		springs.rowsSize++;
	}

	return springs;
}

#define isDefective(x) (x == '?' || x == '#')
#define isWorking(x) (x == '?' || x == '.' || x == '\0')

int64_t matchGroup(char *start, uint32_t *groups, uint32_t groupsSize, uint32_t group){
	size_t matches = 0;

	// after last group, check if consumed all
	if(group >= groupsSize){
		bool ended = true;
		for(size_t pos = 0; start[pos] != '\0'; pos++){
			if(start[pos] == '#') ended = false;
		}

		return ended;
	}

	for(size_t pos = 0; start[pos] != '\0'; pos++){
		size_t i;
		bool match = true;

		// try and see if me can match the defective springs size
		for(i = 0; i < groups[group] && start[pos+i] != '\0'; i++){
			if(!isDefective(start[pos+i])) match = false;
		}

		// if last character was fixed, then we are counting de group size + that character, thus, invalid 
		if(pos > 0 && start[pos - 1] == '#') return matches;
		// stopped before the size of the match
		if(i < groups[group]) match = false;
		// the end should be a working spring or '\0'
		if(!isWorking(start[pos + i])) match = false;

		// on match, try and seek matches ahead
		if(match){
			uint32_t matchesAhead = matchGroup(start + pos + groups[group] + 1, groups, groupsSize, group + 1);

			// if(matchesAhead == 0)
			// 	return matches;

			matches += matchesAhead;
		}
	}

	return matches;
}

void arranges(springs_t *springs){
	for(size_t i = 0; i < springs->rowsSize; i++){
		uint32_t matches = matchGroup(springs->rows[i].springs->raw, springs->rows[i].groups, springs->rows[i].groupsSize, 0);
		springs->rows[i].arrangements = matches;
		springs->arrangements += springs->rows[i].arrangements;
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
	arranges(&springs);

	for(size_t i = 0; i < springs.rowsSize; i++){
		printf("%s ", springs.rows[i].springs->raw);
		for(size_t j = 0; j < springs.rows[i].groupsSize; j++){
			if(j > 0)
				printf(",");

			printf("%u", springs.rows[i].groups[j]);
		}

		printf(" - Arrangements: %lu\n", springs.rows[i].arrangements);
	}

	printf("Part 1: %lu\n", springs.arrangements);

	string_destroy(data);
	return 0;
}