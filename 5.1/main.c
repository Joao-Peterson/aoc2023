#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <regex.h>
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
	uint64_t seedLocation[20];

	map_t map[sectionsMax];
}almanac_t;

// recursively maps the values
uint64_t mapValue(uint64_t value, section_t map, almanac_t *almanac){
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

	// for each seed find the location and store
	for(size_t i = 0; i < almanac.seeds; i++){
		almanac.seedLocation[i] = mapValue(almanac.seed[i], map_seed2soil, &almanac);
	}	
	
	return almanac;
}

// find smallest location
uint64_t smallest(almanac_t *almanac){
	uint64_t ret = UINT64_MAX;

	for(size_t i = 0; i < almanac->seeds; i++){
		if(almanac->seedLocation[i] < ret)
			ret = almanac->seedLocation[i];
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

	almanac_t almanac = parseInput(data);
	printf("%lu\n", smallest(&almanac));

	free(data);
	return 0;
}