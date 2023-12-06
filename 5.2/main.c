#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <regex.h>
#include <pthread.h>
#include "utils.h"

typedef struct{
	size_t size;
	uint64_t range[250][3];
}map_t;

typedef enum{
	section_start = -2,
	section_seeds = -1,
	map_seed2soil,
	map_soil2fertilizer,
	map_fertilizer2water,
	map_water2light,
	map_light2temperature,
	map_temperature2humidity,
	map_humidity2location,
	sectionsMax
}section_t;

typedef struct{
	uint64_t seeds;
	uint64_t seed[20];
	uint64_t lowestLocation;

	map_t map[sectionsMax];
}almanac_t;

// recursively maps the values
uint64_t mapValue(uint64_t value, section_t map, const almanac_t *almanac){
	if(map == sectionsMax) return value;

	uint64_t newValue = value;
	for(size_t i = 0; i <= almanac->map[map].size; i++){
		if(i == almanac->map[map].size){
			break;
		}
		
		uint64_t dest  = almanac->map[map].range[i][0];
		uint64_t src   = almanac->map[map].range[i][1];
		uint64_t range = almanac->map[map].range[i][2];
		
		if(value >= src && value < (src + range)){
			newValue = dest + (value - src);
			break;
		}
	}

	return mapValue(newValue, ++map, almanac);
}

typedef struct{
	size_t start;
	size_t range;
	const almanac_t * almanac;

	size_t lowest;
}threadData_t;

void *threadMapRange(void *data){
	threadData_t *calc = (threadData_t*)data;
	uint64_t lowest = UINT64_MAX;

	for(size_t j = calc->start; j < calc->range + calc->start; j++){
		uint64_t location = mapValue(j, map_seed2soil, calc->almanac);
		if(location < lowest) lowest = location; 
	}

	calc->lowest = lowest;

	return 0;
}

// parse file into almanac_t
almanac_t parseInput(char *data){
	almanac_t almanac = {0};

	section_t section = section_start;
	char *lineState = NULL;
	for(char *line = __strtok_r(data, "\n", &lineState); line != NULL; line = __strtok_r(NULL, "\n", &lineState)){
		
		if(!isdigit(*line)){
			section++;
			continue;
		}

		// seeds
		if(section == section_seeds){

			for(char *numStr = strtok(line, " "); numStr != NULL; numStr = strtok(NULL, " ")){
				almanac.seed[almanac.seeds] = strtoull(numStr, NULL, 10);
				almanac.seeds++;
			}			
		}
		// maps
		else{
			almanac.map[section].range[almanac.map[section].size][0] = strtoull(strtok(line, " "), NULL, 10);
			almanac.map[section].range[almanac.map[section].size][1] = strtoull(strtok(NULL, " "), NULL, 10);
			almanac.map[section].range[almanac.map[section].size][2] = strtoull(strtok(NULL, " "), NULL, 10);
			almanac.map[section].size++;
		}
	}

	// for each seed range, find the lowest location and store
	// fuck, this is inefficient but easy
	// let's try it
	// Result: fucking slow
	// for(size_t i = 0; i < almanac.seeds; i += 2){
	// 	uint64_t start = almanac.seed[i];
	// 	uint64_t range = almanac.seed[i+1];
	// 	uint64_t lowest = UINT64_MAX;

	// 	printf("calc seed!\n");

	// 	for(size_t j = start; j < range + start; j++){
	// 		uint64_t location = mapValue(j, map_seed2soil, &almanac);
	// 		if(location < lowest) lowest = location; 
	// 	}

	// 	almanac.seedLocation[i] = lowest;
	// }	

	pthread_t threads[100];
	threadData_t threadData[100];
	almanac.lowestLocation = UINT64_MAX;

	// for each range spawn thread
	for(size_t i = 0; i < almanac.seeds; i += 2){
		uint64_t start = almanac.seed[i];
		uint64_t range = almanac.seed[i+1];

		threadData[i].almanac = &almanac;
		threadData[i].start = start;
		threadData[i].range = range;

		pthread_create(&(threads[i]), NULL, threadMapRange, &(threadData[i]));
	}
	
	// wait
	// Result: 7m10s total
	for(size_t i = 0; i < almanac.seeds; i += 2){
		pthread_join(threads[i], NULL);
		if(threadData[i].lowest < almanac.lowestLocation)
			almanac.lowestLocation = threadData[i].lowest;
	}

	return almanac;
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

	almanac_t almanac = parseInput(data);
	printf("%lu\n", almanac.lowestLocation);

	free(data);
	return 0;
}