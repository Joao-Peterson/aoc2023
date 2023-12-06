#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <regex.h>
#include "worker.h"
#include "utils.h"

typedef struct{
	uint64_t time[4];
	uint64_t record[4];
}races_t;

// parse file into races_t
races_t parseInput(char *data){
	races_t races = {0};

	int i = 0;
	char *lineState = NULL;
	for(char *line = __strtok_r(data, "\n", &lineState); line != NULL; line = __strtok_r(NULL, "\n", &lineState)){
		if(i == 0){
			strtok(line, " ");
			races.time[0] = strtoull(strtok(NULL, " "), NULL, 10);
			races.time[1] = strtoull(strtok(NULL, " "), NULL, 10);
			races.time[2] = strtoull(strtok(NULL, " "), NULL, 10);
			races.time[3] = strtoull(strtok(NULL, " "), NULL, 10);
			i++;
		}
		else{
			strtok(line, " ");
			races.record[0] = strtoull(strtok(NULL, " "), NULL, 10);
			races.record[1] = strtoull(strtok(NULL, " "), NULL, 10);
			races.record[2] = strtoull(strtok(NULL, " "), NULL, 10);
			races.record[3] = strtoull(strtok(NULL, " "), NULL, 10);
			break;
		}
	}
		
	return races;
}

uint64_t distance(uint64_t buttonTime, uint64_t totalTime){
	uint64_t initial = 0;
	uint64_t vacc = 1;

	return (initial + vacc * ((totalTime * buttonTime) - (buttonTime * buttonTime)));
}

uint64_t winsProd(const races_t *races){
	uint64_t ret = 1;
	
	for(size_t i = 0; i < 4; i++){
		uint64_t wins = 0;
		for(size_t r = 0; r <= races->time[i]; r++){
			uint64_t d = distance(r, races->time[i]);

			if(d > races->record[i])
				wins++;	
		}
		ret *= wins;
	}

	return ret;
}

int main(int argc, char**argv){
	char *data;
	
	// from arg filename
	if(argc > 1){
		data = dataFromFilename(argv[1], NULL);
	}
	// from pipe
	else{
		data = dataFromFile(stdin, NULL);
	}

	races_t races = parseInput(data);
	printf("%lu\n", winsProd(&races));

	free(data);
	return 0;
}