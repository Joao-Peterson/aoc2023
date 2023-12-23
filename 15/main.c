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

void printMatrix(char **m, size_t w, size_t h){
	for(size_t i = 0; i < h; i++){
		printf("%.*s\n", (int)w, m[i]);
	}
}

typedef struct{
	char *label;
	uint8_t focalLength;
}lens_t;

uint8_t hash_string(const char *string){
	uint8_t hash = 0;
	char c = 0;

	while(c = *string++)
		hash = (hash + c) * 17;
		
	return hash;
}

// parse file
void *parseInput(const string *data){

	string_ite_t iterator = string_split(data, ",\n");
	uint32_t i = 0;
	char **steps = calloc(20000, sizeof(char*));
	for(string *step = string_next(&iterator); step != NULL; step = string_next(&iterator)){		
		steps[i] = string_unwrap(step);
		i++;
	}

	steps[i] = NULL;

	return steps;
}

uint64_t part1Sum(char **steps){
	uint64_t acc = 0;
	for(char *step = *steps++; step != NULL; step = *steps++){
		uint8_t hash = hash_string(step);
		acc += hash;
	}
	return acc;
}

void followSteps(char **steps, lens_t boxes[256][100]){
	// for each step
	for(char *step = *steps++; step != NULL; step = *steps++){
		// grab label and lens
		string *stepString = string_wrap(step, false);
		string_ite_t stepIte = string_split(stepString, "=-");

		string *label = string_next(&stepIte);
		string *lens = string_next(&stepIte);
		uint8_t box = hash_string(label->raw);

		// '-' remove lenses from box;
		if(lens == NULL){
			// erase labels
			for(int i = 0; i < 100; i++){
				if(boxes[box][i].label != NULL && !strcmp(boxes[box][i].label, label->raw)){
					// move boxes
					for(int j = i; j < 100 - 1; j++)
						boxes[box][j] = boxes[box][j+1];

					break;
				}
			}
		}
		// '=' add lens to box
		else{
			lens_t new = {
				.label = string_unwrap(label),
				.focalLength = atoi(lens->raw),
			};
			
			for(int i = 0; i < 100; i++){
				if(boxes[box][i].label == NULL || !strcmp(boxes[box][i].label, new.label)){
					boxes[box][i] = new;
					break;
				}
			}

			string_destroy(lens);
		}

		string_destroy(stepString);
	}
}

uint64_t part2Sum(const lens_t boxes[256][100]){
	uint64_t acc = 0;
	for(size_t i = 0; i < 256; i++){
		for(size_t j = 0; j < 100; j++){
			if(boxes[i][j].label != NULL){
				int calc = (i + 1) * (j + 1) * boxes[i][j].focalLength;
				printf("Lens Box[%lu] (%s %u): %d\n", i, boxes[i][j].label, boxes[i][j].focalLength, calc);
				acc += calc;
			}
			else{
				break;
			}
		}
	}
	return acc;
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

	char **steps = parseInput(data);
	printf("Part 1: %lu\n", part1Sum(steps));

	lens_t boxes[256][100] = {0};
	followSteps(steps, boxes);

	printf("Part 2: %lu\n", part2Sum(boxes));

	string_destroy(data);
	return 0;
}