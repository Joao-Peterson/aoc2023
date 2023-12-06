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
	uint64_t time;
	uint64_t record;
}races_t;

// parse file into races_t
races_t parseInput(char *data){
	races_t races = {0};

	int i = 0;
	char *lineState = NULL;
	for(char *line = __strtok_r(data, "\n", &lineState); line != NULL; line = __strtok_r(NULL, "\n", &lineState)){
		char buffer[200];
		if(i == 0){
			buffer[0] = '\0';
			strtok(line, " ");
			strcat(buffer, strtok(NULL, " "));
			strcat(buffer, strtok(NULL, " "));
			strcat(buffer, strtok(NULL, " "));
			strcat(buffer, strtok(NULL, " "));
			races.time = strtoull(buffer, NULL, 10);
			i++;
		}
		else{
			buffer[0] = '\0';
			strtok(line, " ");
			strcat(buffer, strtok(NULL, " "));
			strcat(buffer, strtok(NULL, " "));
			strcat(buffer, strtok(NULL, " "));
			strcat(buffer, strtok(NULL, " "));
			races.record = strtoull(buffer, NULL, 10);
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
	uint64_t wins = 0;

	uint64_t start1 = (races->time / 2);
	uint64_t start2 = start1 - 1;


	// from apex to end 
	for(size_t i = start1; i < races->time; i++){
		if(distance(i, races->time) > races->record)
			wins++;
		else
			break;	
	}

	// from apex to start 
	for(size_t i = start2; i > 0; i--){
		if(distance(i, races->time) > races->record)
			wins++;
		else
			break;	
	}

	// uint64_t d = distance(r, races->time[i]);

	// if(d > races->record[i])
	// 	wins++;	
	
	return wins;
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