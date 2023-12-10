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
	int64_t values[21];
}history_t;

typedef struct{
	history_t sets[200];
	int64_t sum;
}results_t;

int64_t diff(const int64_t *values, size_t size){
	int64_t *buffer = malloc(sizeof(int64_t) * (size - 1));

	bool zeroed = true;
	for(size_t i = 0; i < size - 1; i++){
		buffer[i] = values[i+1] - values[i];
		if(buffer[i] != 0) zeroed = false;
	}

	if(zeroed){
		free(buffer);
		return values[0] - 0;
	}
	else{
		int64_t d = diff(buffer, size - 1);
		free(buffer);
		return values[0] - d;
	}
}

void processResults(results_t *results){
	results->sum = 0;
	for(size_t i = 0; i < 200; i++){
		results->sum += diff(results->sets[i].values, 21);
	}
}

// parse file
results_t parseInput(const string *data){

	results_t results = {0};

	size_t i = 0;
	string_ite_t iterator = string_split(data, "\n");

	for(string *line = string_next(&iterator); line != NULL; line = string_next(&iterator)){
		string_ite_t lineIte = string_split(line, " ");

		size_t j = 0;
		for(string *number = string_next(&lineIte); number != NULL; number = string_next(&lineIte)){
			results.sets[i].values[j] = strtol(number->raw, NULL, 10);
			string_destroy(number);
			j++;
		}

		string_destroy(line);
		i++;
	}

	processResults(&results);
	return results;
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

	results_t results = parseInput(data);
	printf("%ld\n", results.sum);

	string_destroy(data);
	return 0;
}