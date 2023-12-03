#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <regex.h>
#include "utils.h"

typedef struct{
	int red;
	int green;
	int blue;
}play_t;

typedef struct{
	int id;
	int playsSize;
	play_t plays[50];
}game_t;

typedef struct{
	int gameSize;
	game_t games[UINT16_MAX];
	
	regex_t gameRegex;
	regex_t redRegex;
	regex_t greenRegex;
	regex_t blueRegex;
}games_t;

games_t *parseGames(char *file){
	games_t *games = calloc(sizeof(games_t), 1);

	// regex
	
	regcomp(&(games->gameRegex), "Game ([0-9]+)", REG_EXTENDED | REG_NEWLINE | REG_ICASE);
	regcomp(&(games->redRegex), "([0-9]+) red", REG_EXTENDED | REG_NEWLINE | REG_ICASE);
	regcomp(&(games->greenRegex), "([0-9]+) green", REG_EXTENDED | REG_NEWLINE | REG_ICASE);
	regcomp(&(games->blueRegex), "([0-9]+) blue", REG_EXTENDED | REG_NEWLINE | REG_ICASE);

	regmatch_t matches[50];

	for(char *line = strtok(file, "\n"); line != NULL; line = strtok(NULL, "\n")){
		int game = 0;

		// game
		memset(matches, -1, sizeof(regmatch_t)*50);
		if(!regexec(&(games->gameRegex), line, 5, matches, 0)){
			char gameStr[25] = {0};
			memcpy(gameStr, line + matches[1].rm_so, matches[1].rm_eo - matches[1].rm_so); 
			game = atoi(gameStr);
		}

		// only one play with maxs

		// red
		int r = 0;
		int g = 0;
		int b = 0;
		regmatches_t multipleMatches;
		multipleMatches = regexecAll(&(games->redRegex), line);
		if(multipleMatches.size > 0){
			for(int i = 0; i < multipleMatches.size; i++){
				char valueStr[25] = {0};
				memcpy(valueStr, line + multipleMatches.matches[i][1].rm_so, multipleMatches.matches[i][1].rm_eo - multipleMatches.matches[i][1].rm_so); 
				int value = atoi(valueStr);				

				if(value > r) r = value;
			}
		}
		multipleMatches = regexecAll(&(games->greenRegex), line);
		if(multipleMatches.size > 0){
			for(int i = 0; i < multipleMatches.size; i++){
				char valueStr[25] = {0};
				memcpy(valueStr, line + multipleMatches.matches[i][1].rm_so, multipleMatches.matches[i][1].rm_eo - multipleMatches.matches[i][1].rm_so); 
				int value = atoi(valueStr);				

				if(value > g) g = value;
			}
		}
		multipleMatches = regexecAll(&(games->blueRegex), line);
		if(multipleMatches.size > 0){
			for(int i = 0; i < multipleMatches.size; i++){
				char valueStr[25] = {0};
				memcpy(valueStr, line + multipleMatches.matches[i][1].rm_so, multipleMatches.matches[i][1].rm_eo - multipleMatches.matches[i][1].rm_so); 
				int value = atoi(valueStr);				

				if(value > b) b = value;
			}
		}

		games->games[games->gameSize].id = game;
		games->games[games->gameSize].plays[0].red = r;
		games->games[games->gameSize].plays[0].green = g;
		games->games[games->gameSize].plays[0].blue = b;
		games->games[games->gameSize].playsSize++;
		games->gameSize++;

		printf("game %d: (%d, %d, %d)\n", game, r, g, b);
	}

	return games;
}

void games_destroy(games_t *games){
	regfree(&(games->gameRegex));
	regfree(&(games->redRegex));
	regfree(&(games->greenRegex));
	regfree(&(games->blueRegex));
	free(games);
}

uint64_t sumOf(games_t *games, int r, int g, int b){
	uint64_t acc = 0;
	
	for(int i = 0; i < games->gameSize; i++){
		if(
			(games->games[i].plays[0].red <= r) &&
			(games->games[i].plays[0].green <= g) &&
			(games->games[i].plays[0].blue <= b)
		){
			acc += games->games[i].id;
			printf("possible game %d: (%d, %d, %d)\n", games->games[i].id, games->games[i].plays[0].red, games->games[i].plays[0].green, games->games[i].plays[0].blue);
		}
	}

	return acc;
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

	games_t *games = parseGames(data);
	printf("%lu\n", sumOf(games, 12, 13, 14));

	games_destroy(games);
	free(data);
	return 0;
}