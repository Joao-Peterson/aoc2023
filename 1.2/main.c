#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include "utils.h"
#include "strmatch.h"

uint64_t sumCals(char *calibrations, strmatch_t *sm);

int main(int argc, char**argv){
	char *data;
	
	// from arg filename
	if(argc > 1){
		data = filetomem(argv[1], NULL);
	}
	// from pipe
	else{
		data = malloc(1024*1000);
		char buffer[1024];
		*data = '\0';
		*buffer = '\0';
		while(fgets(buffer, 1024, stdin) != NULL){
			strcat(data, buffer);
		}
	}

	strmatch_t *sm = strmatch_new(20, 
		"zero", 0, "one", 1, "two", 2, "three", 3, "four", 4, "five", 5, "six", 6, "seven", 7, "eight", 8, "nine", 9,
		"0", 0, "1", 1, "2", 2, "3", 3, "4", 4, "5", 5, "6", 6, "7", 7, "8", 8, "9", 9
	);

	printf("%lu", sumCals(data, sm));

	strmatch_destroy(sm);
	free(data);
	return 0;
}

uint64_t sumCals(char *calibrations, strmatch_t *sm){
	int first = -1;
	int last = -1;
	uint16_t acc = 0;

	for(char *line = strtok(calibrations, "\n"); line != NULL; line = strtok(NULL, "\n")){
		size_t l = strlen(line);
		for(size_t i = 0; i < l; i++){
			// match against every char on line
			charnode_t *match = strmatch_match_atstart(sm, &(line[i]));

			if(match){
				last = match->value;
				if(first == -1) 
					first = last;
			}
		}
		
		acc += first * 10 + last;
		first = -1;
		last = -1;
	}

	return acc;
}
