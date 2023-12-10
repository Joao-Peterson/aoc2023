#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <regex.h>
#include "worker.h"
#include "utils.h"

typedef enum{
	rank_high_card,
	rank_1_pair,
	rank_2_pair,
	rank_3_of_a_kind,
	rank_full_house,
	rank_4_of_a_kind,
	rank_5_of_a_kind,
}rank_t;

typedef struct{
	char cards[5];
	uint64_t rank;
	uint64_t bid;
}hand_t;

typedef struct{
	hand_t hands[1000];
	uint64_t win;
}play_t;

int cardStrength(char card){
	switch(card){
		case 'A':
			return 12;
		case 'K':
			return 11;
		case 'Q':
			return 10;
		case 'J':
			return 9;
		case 'T':
			return 8;
		case '9':
			return 7;
		case '8':
			return 6;
		case '7':
			return 5;
		case '6':
			return 4;
		case '5':
			return 3;
		case '4':
			return 2;
		case '3':
			return 1;
		default:
		case '2':
			return 0;
	}
}

rank_t handRank(char const hand[5]){
	int qty[13] = {0};

	// count
	for(int i = 0; i < 5; i++){
		// increment qty of card in buffer on position equakl to strength, strengh being like an index 
		qty[cardStrength(hand[i])]++;
	}

	int max = 0;
	int maxQty = 0;
	
	// find max
	for(int i = 0; i < 13; i++){
		if(qty[i] > maxQty){
			maxQty = qty[i];
			max = i; 
		}
	}

	int max2 = 0;
	int max2Qty = 0;

	// find second max
	for(int i = 0; i < 13; i++){
		if(qty[i] > max2Qty && i != max){
			max2Qty = qty[i];
			max2 = i; 
		}
	}

	switch(maxQty){
		case 5:
			return rank_5_of_a_kind;
		
		case 4:
			return rank_4_of_a_kind;
		
		case 3:
			if(max2Qty == 2)
				return rank_full_house;
			else
				return rank_3_of_a_kind;

		case 2:
			if(max2Qty == 2)
				return rank_2_pair;
			else
				return rank_1_pair;

		default:
		case 1:
			return rank_high_card; 
	}
}

// qsort comp
int orderHands(const void *a, const void *b){
	const hand_t *handA = (hand_t*)a;
	const hand_t *handB = (hand_t*)b;

	// bigger card
	if(handA->rank == handB->rank){
		for(size_t i = 0; i < 5; i++){
			// return the bigger card, otherwise continue
			int cardA = cardStrength(handA->cards[i]);
			int cardB = cardStrength(handB->cards[i]);

			if(cardA != cardB)
				return cardA - cardB;
		}

		// otherwise return first
		return 1;
	}
	// bigger rank
	else{
		return handA->rank - handB->rank;
	}
}

// calculate winnings
void winnings(play_t *play){
	// sort
	qsort(play->hands, 1000, sizeof(hand_t), orderHands);

	play->win = 0;

	for(size_t i = 0; i < 1000; i++){
		uint64_t a = (i + 1);
		uint64_t b = a * play->hands[i].bid;
		play->win += b;
	}
}

// parse file
play_t parseInput(char *data){

	play_t play = {0};

	int i = 0;
	char *lineState = NULL;
	for(char *line = __strtok_r(data, "\n", &lineState); line != NULL; line = __strtok_r(NULL, "\n", &lineState)){
		memcpy(play.hands[i].cards, strtok(line, " "), 5);
		play.hands[i].rank = handRank(play.hands[i].cards);
		play.hands[i].bid = strtoull(strtok(NULL, " "), NULL, 10);
		i++;
	}
		
	winnings(&play);
	return play;
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

	play_t play = parseInput(data);
	uint64_t win = 0;
	for(int i = 0; i < 1000; i++){
		uint64_t a = (i + 1);
		uint64_t b = a * play.hands[i].bid;
		win += b;
		
		printf("Hand [%d]: %c%c%c%c%c - bid: %llu - total: %llu\n", i,
			play.hands[i].cards[0],
			play.hands[i].cards[1],
			play.hands[i].cards[2],
			play.hands[i].cards[3],
			play.hands[i].cards[4],
			play.hands[i].bid,
			win
		);
	}
	// printf("%lu\n", play.win);

	free(data);
	return 0;
}