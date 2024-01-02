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
#include "src/data.h"

void printMatrix(char **m, size_t w, size_t h){
	for(size_t i = 0; i < h; i++){
		printf("%.*s\n", (int)w, m[i]);
	}
}

// parse file
void *parseInput(const string *data){

	string_ite_t iterator = string_split(data, "\n");
	for(string *line = string_next(&iterator); line != NULL; line = string_next(&iterator)){
		string_destroy(line);
	}

	return ;
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

	//  = parseInput(data);
	
	// printf("Part 1: %lu\n", );
	// printf("Part 2: %lu\n", );

	string_destroy(data);
	return 0;
}