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

// mapped nodes
typedef struct{
	uint32_t left;
	uint32_t right;
}node_list_entry_t;

typedef struct{
	string *directions;
	node_list_entry_t entries[17576]; // 0 -> AAA; 17575 -> ZZZ
}node_list_t;

uint32_t node_list_key(const char name[4]){
	number_t key26 = number_from_string_alphabet(name);
	number_t key10 = number_convert(key26, 10);
	return number_to_uint(key10);
}

void runList(node_list_t *list, uint32_t from, uint32_t to, char *dir, uint64_t *acc){
	uint32_t next;
	
	if(dir == NULL || *dir == '\0')
		dir = list->directions->raw;


	if(*dir == 'L')
		next = list->entries[from].left;
	else
		next = list->entries[from].right;

	//! debug
	// number_t name10 = number_from_uint(from);
	// number_t name26 = number_convert(name10, 26);
	// char *nameStr = number_to_string_alphabet(name26);

	// number_t next10 = number_from_uint(next);
	// number_t next26 = number_convert(next10, 26);
	// char *nextStr = number_to_string_alphabet(next26);

	// printf("Dir: %s\n%s = (%s)\n", dir, nameStr, nextStr);
	// free(nameStr);
	// free(nextStr);

	if(!(next == to))
		runList(list, next, to, ++dir, acc);

	(*acc)++;
}

// parse file
node_list_t parseInput(const string *data){

	node_list_t map = {0};

	int i = 0;
	string_ite_t iterator = string_split(data, "\n");

	// list all nodes
	for(string *line = string_next(&iterator); line != NULL; line = string_next(&iterator)){
		if(i == 0){
			map.directions = line;
		}
		else{
			char buffer[4] = {0};
			
			// list key
			memcpy(buffer, line->raw + 0, 3);
			uint32_t key = node_list_key(buffer);

			// left
			memcpy(buffer, line->raw + 7, 3);
			map.entries[key].left = node_list_key(buffer);

			// right
			memcpy(buffer, line->raw + 12, 3);
			map.entries[key].right = node_list_key(buffer);

			string_destroy(line);
		}

		i++;
	}

	return map;
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

	node_list_t map = parseInput(data);
	uint64_t d = 0;
	
	runList(&map,
		node_list_key("AAA"),
		node_list_key("ZZZ"),
		map.directions->raw,
		&d
	);

	printf("%lu\n", d);

	string_destroy(map.directions);
	string_destroy(data);
	return 0;
}