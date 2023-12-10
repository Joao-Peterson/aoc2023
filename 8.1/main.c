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
	char left[3];
	char right[3];
}node_map_entry_t;

typedef struct{
	char directions[263];
	node_map_entry_t entries[21175]; // 21175 -> ZZZ
}node_map_t;

// typedef struct node_t note_t;

// struct node_t{
// 	node_t *left;
// 	node_t *right;
// };

// parse file
node_map_t parseInput(const string *data){

	node_map_t map = {0};
	int i = 0;
	string_ite_t iterator = string_split(data, "\n");
	for(string *line = string_next(&iterator); line != NULL; line = string_next(&iterator)){
		if(i == 0){
			memcpy(map.directions, line->raw, 263);
		}
		else{
			char nameBuffer[4] = {0};
			memcpy(nameBuffer, line->raw + 0, 3);

			number_t key26 = number_from_string_alphabet(nameBuffer);
			number_t key10 = number_convert(key26, 10);
			size_t key = number_to_uint(key10);

			memcpy(map.entries[key].left, line->raw + 7, 3);
			memcpy(map.entries[key].right, line->raw + 12, 3);
		}

		i++;
		string_destroy(line);
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

	node_map_t map = parseInput(data);
	// printf("%lu\n", play.win);

	string_destroy(data);
	return 0;
}