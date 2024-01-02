#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <regex.h>
#include <math.h>
#include "src/worker.h"
#include "src/utils.h"
#include "src/string+.h"
#include "src/number.h"
#include "src/hash.h"
#include "src/data.h"
#include "src/flood_fill.h"

void printMatrix(char **m, size_t w, size_t h){
	for(size_t i = 0; i < h; i++){
		printf("%.*s\n", (int)w, m[i]);
	}
}

typedef enum{
	dir_up = 'U',
	dir_down = 'D',
	dir_left = 'L',
	dir_right = 'R',
}dir_t;

typedef struct{
	dir_t dir;
	int64_t distance;
	uint8_t color[3];
}dig_t;

typedef struct{
	dig_t dig[700];
	int64_t points[701][2];
	int size;
	int64_t w;
	int64_t h;
	int64_t offw;
	int64_t offh;
	int64_t wmin;
	int64_t hmin;
	int **map;
	int **biggerMap;
}plan_t;

uint8_t hex2int(char c){
	if(c >= '0' && c <= '9')
		return c - '0';
	else if(c >= 'a' && c <= 'z')
		return c - 'a';
	else if(c >= 'A' && c <= 'Z')
		return c - 'A';
	else 
		return 0;
}

void colorParse(const string *s, uint8_t color[3]){
	// (#7807d2)
	color[0] = (hex2int(s->raw[2]) << 4) | hex2int(s->raw[3]);
	color[1] = (hex2int(s->raw[4]) << 4) | hex2int(s->raw[5]);
	color[2] = (hex2int(s->raw[6]) << 4) | hex2int(s->raw[7]);
}

int distanceParse(const string *s){
	char buffer[6] = {0};
	memcpy(buffer, s->raw + 2, 5);
	return strtol(buffer, NULL, 16);
}

dir_t dirParse(const string *s){
	switch(s->raw[7]){
		default:
		case '0':
			return dir_right;
		case '1':
			return dir_down;
		case '2':
			return dir_left;
		case '3':
			return dir_up;
	}
}

const uint8_t white[3] = {255, 255, 255};
const uint8_t red[3] = {255, 0, 0};
const uint8_t magenta[3] = {255, 0, 255};
const uint8_t blue[3] = {0, 0, 255};
const uint8_t cyan[3] = {0, 255, 255};
const uint8_t green[3] = {0, 255, 0};
const uint8_t yellow[3] = {255, 255, 0};
const uint8_t gray[3] = {124, 124, 124};
const uint8_t black[3] = {0, 0, 0};

void printColorCell(const uint8_t color[3]){
	printf("\x1b[48;2;%d;%d;%dm" " " "\x1b[m", color[0], color[1], color[2]);
}

// parse file
void *parseInput(const string *data){

	plan_t *plan = calloc(1, sizeof(plan_t));
	plan->h = 0;
	plan->w = 0;
	int h = 0;
	int w = 0;

	string_ite_t iterator = string_split(data, "\n");
	for(string *line = string_next(&iterator); line != NULL; line = string_next(&iterator)){
		string_ite_t lineSplit = string_split(line, " ");

		string *dirStr = string_next(&lineSplit);
		string *distanceStr = string_next(&lineSplit);
		string *colorStr = string_next(&lineSplit);

		plan->dig[plan->size].dir = (dir_t)dirStr->raw[0];
		plan->dig[plan->size].distance = strtod(distanceStr->raw, NULL);
		colorParse(colorStr, plan->dig[plan->size].color);

		// max and min amount in each axis
		switch(plan->dig[plan->size].dir){
			case dir_down:	
				h += plan->dig[plan->size].distance;
				if(h > plan->h)
					plan->h = h;
				if(h < plan->hmin)
					plan->hmin = h;
			break;
			case dir_up:	
				h -= plan->dig[plan->size].distance;
				if(h > plan->h)
					plan->h = h;
				if(h < plan->hmin)
					plan->hmin = h;
			break;
			case dir_right:	
				w += plan->dig[plan->size].distance;
				if(w > plan->w)
					plan->w = w;
				if(w < plan->wmin)
					plan->wmin = w;
			break;
			case dir_left:	
				w -= plan->dig[plan->size].distance;
				if(w > plan->w)
					plan->w = w;
				if(w < plan->wmin)
					plan->wmin = w;
			break;

			default:
			break;
		}

		plan->size++;

		string_destroy(dirStr);
		string_destroy(distanceStr);
		string_destroy(colorStr);
		string_destroy(line);
	}

	plan->offw = -plan->wmin;
	plan->w += plan->offw + 1;

	plan->offh = -plan->hmin;
	plan->h += plan->offh + 1;

	plan->map = calloc(plan->h, sizeof(int*));
	for(int i = 0; i < plan->h; i++){
		plan->map[i] = calloc(plan->w, sizeof(int));
		for(int j = 0; j < plan->w; j++){
			plan->map[i][j] = -1;
		}
	}

	return plan;
}

// parse file
void *parseInput2(const string *data){

	plan_t *plan = calloc(1, sizeof(plan_t));
	plan->h = 0;
	plan->w = 0;
	int64_t h = 0;
	int64_t w = 0;

	string_ite_t iterator = string_split(data, "\n");
	for(string *line = string_next(&iterator); line != NULL; line = string_next(&iterator)){
		string_ite_t lineSplit = string_split(line, " ");

		string *dirStr = string_next(&lineSplit);
		string *distanceStr = string_next(&lineSplit);
		string *colorStr = string_next(&lineSplit);

		plan->dig[plan->size].dir = dirParse(colorStr);
		plan->dig[plan->size].distance = distanceParse(colorStr);
		colorParse(colorStr, plan->dig[plan->size].color);

		plan->size++;

		string_destroy(dirStr);
		string_destroy(distanceStr);
		string_destroy(colorStr);
		string_destroy(line);
	}

	return plan;
}

void printMap(const plan_t *plan){
	for(int i = 0; i < plan->h + 2; i++){
		for(int j = 0; j < plan->w + 2; j++){
			if(plan->biggerMap[i][j] == -2)
				printColorCell(white);
			// else if(i == (0 + 1 + plan->offh) && (j == 0 + 1 + plan->offw))
			// 	printColorCell(yellow);
			else if(plan->biggerMap[i][j] == -1)
				printColorCell(gray);
			else if(plan->biggerMap[i][j] >= 0)
				printColorCell(black);
				// printColorCell(plan->dig[plan->biggerMap[i][j]].color);
			else
				printColorCell(yellow);


		}
		printf("\n");
	}
}

uint64_t part1(plan_t *plan){
	int x = 0, y = 0;
	plan->map[y+plan->offh][x+plan->offw] = 0;
	for(int i = 0; i < plan->size; i++){
		// printf("%c %d (#%02x%02x%02x)\n", plan->dig[i].dir, plan->dig[i].distance, plan->dig[i].color[0], plan->dig[i].color[1], plan->dig[i].color[2]);
		
		for(int d = 0; d < plan->dig[i].distance; d++){
			switch(plan->dig[i].dir){
				case dir_left:
					// if(x-1 >= 0) x -= 1;
					x -= 1;
				break;
				case dir_right:
					// if(x+1 < plan->w) x += 1;
					x += 1;
				break;
				case dir_up:
					// if(y-1 >= 0) y -= 1;
					y -= 1;
				break;
				case dir_down:
					// if(y+1 < plan->h) y += 1;
					y += 1;
				break;
			}

			plan->map[y+plan->offh][x+plan->offw] = i;
		}
	}

	plan->biggerMap = malloc(sizeof(int*) * (plan->h + 2));
	for(int i = 0; i < plan->h + 2; i++){
		plan->biggerMap[i] = malloc(sizeof(int) * (plan->w + 2));
		for(int j = 0; j < plan->w + 2; j++){
			if(i > 0 && i <= plan->h && j > 0 && j <= plan->w){
				plan->biggerMap[i][j] = plan->map[i-1][j-1];
			}
			else{
				plan->biggerMap[i][j] = -1;
			}
		}
	}

	// printMap(plan);

	flood_fill_int(plan->biggerMap, plan->h + 2, plan->w + 2, 0, 0, -1, -2);
	
	printMap(plan);

	uint64_t acc = 0;
	for(int i = 0; i < plan->h + 2; i++){
		for(int j = 0; j < plan->w + 2; j++){
			if(plan->biggerMap[i][j] != -2)
				acc++;
		}
	}

	return acc;
}

int64_t *pointAt(plan_t *plan, int64_t i){
	if(i < 0)
		return plan->points[i + (plan->size + 1)];
	else if(i >= (plan->size + 1))
		return plan->points[i - (plan->size + 1)];
	else
		return plan->points[i];
}

uint64_t part2(plan_t *plan){
	int64_t area = 0;
	int64_t p = 1;

	plan->points[0][0] = 0;
	plan->points[0][1] = 0;
	for(int i = 0; i < plan->size; i++){
		plan->points[i+1][0] = plan->points[i][0];
		plan->points[i+1][1] = plan->points[i][1];
		p += plan->dig[i].distance;
		switch(plan->dig[i].dir){
			case dir_left:
				plan->points[i+1][1] -= plan->dig[i].distance;
			break;
			case dir_right:
				plan->points[i+1][1] += plan->dig[i].distance;
			break;
			case dir_up:
				plan->points[i+1][0] -= plan->dig[i].distance;
			break;
			case dir_down:
				plan->points[i+1][0] += plan->dig[i].distance;
			break;
		}
	}

	// https://en.wikipedia.org/wiki/Shoelace_formula
	for(int64_t i = 0; i < plan->size + 1; i++)
		area += pointAt(plan, i)[1] * (pointAt(plan, i+1)[0] - pointAt(plan, i-1)[0]);

	area = labs(area / 2);

	// https://en.wikipedia.org/wiki/Pick's_theorem
	return area + 1 + p/2;
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
	
	plan_t *plan = parseInput(data);
	printf("Part 1 floodfill: %lu\n", part1(plan));
	printf("Part 1 shoelace + pick's: %lu\n", part2(plan));
	// too low 11574
	// too high 36808

	plan_t *plan2 = parseInput2(data);
	printf("Part 2: %lu\n", part2(plan2));

	string_destroy(data);
	return 0;
}