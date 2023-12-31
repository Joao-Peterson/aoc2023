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

void printStringMatrix(const char **m, size_t w, size_t h){
	for(size_t i = 0; i < h; i++){
		printf("%.*s\n", (int)w, m[i]);
	}
}

void printStringNumMatrix(char **m, size_t w, size_t h, bool commaSep){
	for(size_t i = 0; i < h; i++){
		for(size_t j = 0; j < w; j++){
			if(commaSep && j > 0)
				printf(", ");

			printf("%d", m[i][j]);
		}
		printf("\n");
	}
}

void printNumMatrix(int **m, size_t w, size_t h, bool commaSep){
	for(size_t i = 0; i < h; i++){
		for(size_t j = 0; j < w; j++){
			if(commaSep && j > 0)
				printf(", ");

			printf("%d", m[i][j]);
		}
		printf("\n");
	}
}

typedef struct{
	char **heatLoss;
	int w;
	int h;
}rails_t;

typedef struct point_t point_t;
struct point_t{
	int x;
	int y;
	int dirx;
	int diry;
	int travel;
	int heat;
	point_t *next;
};

bool point_cmp(void *a, void *b){
	point_t *pa = (point_t*)a;
	point_t *pb = (point_t*)b;

	return pa->heat < pb->heat;
}

void point_push(pqueue_t *pq, int heat, int x, int y, int dx, int dy, int travel){
	point_t *p = malloc(sizeof(point_t));
	p->heat = heat;
	p->x = x;
	p->y = y;
	p->dirx = dx;
	p->diry = dy;
	p->travel = travel;

	pqueue_push(pq, p);
}

point_t *point_pop(pqueue_t *pq){
	return (point_t*)pqueue_pop(pq);
}

bool inside(int y, int x, const rails_t *r){
	return !(x < 0 || x > r->w - 1 || y < 0 || y > r->h - 1);
}

uint64_t part1(const rails_t *rails){
	pqueue_t *q = pqueue_new(point_cmp, true);
	// 141 * 141 area, out of wich, each cell could have different neirbour states of the 4 different directions with 3 different travel distances that enables 3 adjacent cells to move in each direction, so 4*3 states
	// 141*141*4*3 maximum possible states
	set_t *seen = set_new(141*141*4*3);
	
	int visited[141][141][2] = {0};
	memset(visited, -1, sizeof(int)*141*141*2);
	int losses[141][141] = {0};
	for(int i = 0; i < 141; i++)
		for(int j = 0; j < 141; j++)
			losses[i][j] = 100000;

	char path[141][141] = {0};
	memset(path, ' ', sizeof(char)*141*141);
	
	uint64_t min = 0;

	// start
	point_push(q, 0, 0, 0, 1, 0, 0);

	// consume less heated points
	for(point_t *p = point_pop(q); p != NULL; p = point_pop(q)){
		
		if(set_exists(seen, p, ((void*)(&p->travel + 1) - (void*)&p->x)))
			continue;

		set_add(seen, p, ((void*)(&p->travel + 1) - (void*)&p->x));

		if(p->heat < losses[p->y][p->x]){
			visited[p->y][p->x][0] = p->y - p->diry;
			visited[p->y][p->x][1] = p->x - p->dirx;
			losses[p->y][p->x] = p->heat;	
		}

		// end on bottom right
		if(p->x == rails->w - 1 && p->y == rails->h - 1){
			min = p->heat; 
			break;
		}

		int nx;
		int ny;
		int dx;
		int dy;

		// ahead
		dx = p->dirx;
		dy = p->diry;
		nx = p->x + dx;
		ny = p->y + dy;
		if(inside(ny, nx, rails) && p->travel < 3){
			point_push(q, 
				p->heat + rails->heatLoss[ny][nx],
				nx, ny,
				dx, dy,
				p->travel + 1
			);		
		}

		// adjacent side
		dx = p->diry;
		dy = p->dirx;
		nx = p->x + dx;
		ny = p->y + dy;
		if(inside(ny, nx, rails)){
			point_push(q, 
				p->heat + rails->heatLoss[ny][nx],
				nx, ny,
				dx, dy,
				1
			);		
		}

		// other adjacent side
		dx = - p->diry;
		dy = - p->dirx;
		nx = p->x + dx;
		ny = p->y + dy;
		if(inside(ny, nx, rails)){
			point_push(q, 
				p->heat + rails->heatLoss[ny][nx],
				nx, ny,
				dx, dy,
				1
			);		
		}
	}

	printf("Heatloss:\n");
	printStringNumMatrix(rails->heatLoss, rails->w, rails->h, false);
	printf("Losses:\n");
	for(int i = 0; i < rails->h; i++){
		for(int j = 0; j < rails->w; j++){
			if(j > 0)
				printf(", ");

			printf("%d", losses[i][j]);
		}
		printf("\n");
	}

	printf("From:\n");
	for(int i = 0; i < rails->h; i++){
		for(int j = 0; j < rails->w; j++){
			if(j > 0)
				printf(", ");

			if(visited[i][j][0] == -1 && visited[i][j][1] == -1)
				printf("[%c,%c]", '#', '#');
			else
				printf("[%d,%d]", visited[i][j][0], visited[i][j][1]);
		}
		printf("\n");
	}

	int tx = rails->w - 1, ty = rails->h - 1;
	int i = 0;
	for(int x = tx, y = ty; x > -1 && y > -1; tx = visited[y][x][1], ty = visited[y][x][0], x = tx, y = ty){
		path[y][x] = '#';
		i++;

		// anti blowup
		if(i > 1000)
			break;
	}

	printf("Path:\n");
	for(i = 0; i < rails->h; i++){
		printf("%.*s\n", (int)rails->w, path[i]);
	}

	pqueue_destroy(q);
	set_destroy(seen);
	return min;
}

uint64_t part2(const rails_t *rails){
	pqueue_t *q = pqueue_new(point_cmp, true);
	// 141 * 141 area, out of wich, each cell could have different neirbour states of the 4 different directions with 3 different travel distances that enables 3 adjacent cells to move in each direction, so 4*3 states
	// 141*141*4*3 maximum possible states
	// had to bump set size here, dont know why :P
	set_t *seen = set_new(141*141*4*10);
	
	int visited[141][141][2] = {0};
	memset(visited, -1, sizeof(int)*141*141*2);
	int losses[141][141] = {0};
	for(int i = 0; i < 141; i++)
		for(int j = 0; j < 141; j++)
			losses[i][j] = 100000;

	char path[141][141] = {0};
	memset(path, ' ', sizeof(char)*141*141);
	
	uint64_t min = 0;

	// start
	point_push(q, 0, 0, 0, 1, 0, 0);

	// consume less heated points
	for(point_t *p = point_pop(q); p != NULL; p = point_pop(q)){
		
		if(set_exists(seen, p, ((void*)(&p->travel + 1) - (void*)&p->x)))
			continue;

		set_add(seen, p, ((void*)(&p->travel + 1) - (void*)&p->x));

		if(p->heat < losses[p->y][p->x]){
			visited[p->y][p->x][0] = p->y - p->diry;
			visited[p->y][p->x][1] = p->x - p->dirx;
			losses[p->y][p->x] = p->heat;	
		}

		// end on bottom right
		if(p->x == rails->w - 1 && p->y == rails->h - 1){
			min = p->heat; 
			break;
		}

		int nx;
		int ny;
		int dx;
		int dy;

		// ahead
		dx = p->dirx;
		dy = p->diry;
		nx = p->x + dx;
		ny = p->y + dy;
		if(inside(ny, nx, rails) && p->travel < 10){
			point_push(q, 
				p->heat + rails->heatLoss[ny][nx],
				nx, ny,
				dx, dy,
				p->travel + 1
			);		
		}

		// adjacent side
		dx = p->diry;
		dy = p->dirx;
		nx = p->x + dx;
		ny = p->y + dy;
		if(inside(ny, nx, rails) && (p->travel >= 4 || (p->x == 0 && p->y == 0))){
			point_push(q, 
				p->heat + rails->heatLoss[ny][nx],
				nx, ny,
				dx, dy,
				1
			);		
		}

		// other adjacent side
		dx = - p->diry;
		dy = - p->dirx;
		nx = p->x + dx;
		ny = p->y + dy;
		if(inside(ny, nx, rails) && (p->travel >= 4 || (p->x == 0 && p->y == 0))){
			point_push(q, 
				p->heat + rails->heatLoss[ny][nx],
				nx, ny,
				dx, dy,
				1
			);		
		}
	}

	printf("Heatloss:\n");
	printStringNumMatrix(rails->heatLoss, rails->w, rails->h, false);
	printf("Losses:\n");
	for(int i = 0; i < rails->h; i++){
		for(int j = 0; j < rails->w; j++){
			if(j > 0)
				printf(", ");

			printf("%d", losses[i][j]);
		}
		printf("\n");
	}

	printf("From:\n");
	for(int i = 0; i < rails->h; i++){
		for(int j = 0; j < rails->w; j++){
			if(j > 0)
				printf(", ");

			if(visited[i][j][0] == -1 && visited[i][j][1] == -1)
				printf("[%c,%c]", '#', '#');
			else
				printf("[%d,%d]", visited[i][j][0], visited[i][j][1]);
		}
		printf("\n");
	}

	int tx = rails->w - 1, ty = rails->h - 1;
	int i = 0;
	for(int x = tx, y = ty; x > -1 && y > -1; tx = visited[y][x][1], ty = visited[y][x][0], x = tx, y = ty){
		path[y][x] = '#';
		i++;

		// anti blowup
		if(i > 1000)
			break;
	}

	printf("Path:\n");
	for(i = 0; i < rails->h; i++){
		printf("%.*s\n", (int)rails->w, path[i]);
	}

	pqueue_destroy(q);
	set_destroy(seen);
	return min;
}

// parse file
void *parseInput(const string *data){

	string_ite_t iterator = string_split(data, "\n");
	rails_t *rails = calloc(1, sizeof(rails_t));
	rails->heatLoss = calloc(150, sizeof(char*));

	for(string *line = string_next(&iterator); line != NULL; line = string_next(&iterator)){
		rails->w = line->len;

		for(size_t i = 0; i < line->len; i++)
			line->raw[i] -= '0';
			
		rails->heatLoss[rails->h] = string_unwrap(line);
		rails->h++;
	}

	return rails;
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

	rails_t *rails = parseInput(data);
	printf("Part 1: %lu\n", part1(rails));
	printf("Part 2: %lu\n", part2(rails));

	free(rails); 
	string_destroy(data);
	return 0;
}