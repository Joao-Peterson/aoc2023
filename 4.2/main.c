#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <regex.h>
#include "utils.h"

typedef struct{
	int win [10];
	int num[25];
	int matches;
	int qty;
}card_t;

typedef struct{
	size_t size;
	card_t at[500];
}cards_t;

cards_t parseInput(char *data){
	char *token = strtok(data, " :|\n");
	cards_t cards = {.size = 0};

	for(size_t i = 0; token != NULL; i++){
		cards.at[cards.size].matches = 0;
		cards.at[cards.size].qty = 1;
		
		if(i >= 2 && i < 12){
			cards.at[cards.size].win[i-2] = atoi(token); 
		}
		else if(i >= 12 && i < 37){
			cards.at[cards.size].num[i-12] = atoi(token); 
		}
		else if(i >= 37){
			cards.size++;
			i = 0;
		}

		token = strtok(NULL, " :|\n");
	}

	cards.size++;

	return cards;
}

uint64_t winners(cards_t cards){
	// comp matches
	for(size_t i = 0; i < cards.size; i++){
		for(size_t j = 0; j < 25; j++){
			for(size_t k = 0; k < 10; k++){
				if(cards.at[i].num[j] == cards.at[i].win[k]){
					cards.at[i].matches++;
				}
			}
		}
	}

	uint64_t total = 0;

	// comp copies
	for(size_t i = 0; i < cards.size; i++){
		printf("Card %d. Matches: %d. Qty: %d\n", (int)i, cards.at[i].matches, cards.at[i].qty);
		total += cards.at[i].qty;
		
		// increment quantity of each following card
		for(size_t j = i+1; (j < i + 1 + cards.at[i].matches) && (j < cards.size); j++)
			cards.at[j].qty += cards.at[i].qty;
	}

	return total;
}

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

	cards_t cards = parseInput(data);
	printf("%lu\n", winners(cards));

	free(data);
	return 0;
}