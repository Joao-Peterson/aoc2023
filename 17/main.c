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

typedef struct{
	point_t *first;
	point_t *last;
	size_t size;
}pqueue_t;

void pqueue_push(pqueue_t *q, int heat, int x, int y, int dirx, int diry, int travel){
	point_t *p = calloc(1, sizeof(point_t));
	p->y = y;
	p->x = x;
	p->heat = heat;
	p->next = NULL;
	p->dirx = dirx;
	p->diry = diry;
	p->travel = travel;
	// p->fromy = fromy;
	// p->fromx = fromx;

	if(q->size == 0){
		q->first = p;
		q->last = p;
		q->size++;
	}
	else{
		point_t *cursor;
		for(cursor = q->first; cursor != NULL; cursor = cursor->next){
			if(p->heat < cursor->heat){
				q->first = p;
				p->next = cursor;
				q->size++;
				break;
			}
			else{
				if(cursor->next == NULL){
					q->last = p;
					cursor->next = p;
					q->size++;
					break;	
				}
				else if(p->heat < cursor->next->heat){
					p->next = cursor->next;
					cursor->next = p;
					q->size++;
					break;	
				}

			}
		}
	}
}

point_t *pqueue_pop(pqueue_t *q){
	if(q->size == 0)
		return NULL;

	if(q->size == 1){
		point_t *p = q->first;
		q->first = NULL;
		q->last = NULL;
		q->size = 0;
		return p;
	}

	point_t *p = q->first;
	q->first = p->next;
	q->size--;
	return p;
}

bool inside(int y, int x, const rails_t *r){
	return !(x < 0 || x > r->w - 1 || y < 0 || y > r->h - 1);
}

uint64_t part1(const rails_t *rails){
	pqueue_t q = {0};
	// 141 * 141 area, out of wich, each cell could have different neirbour states of the 4 different directions with 3 different travel distances 
	// 141*141*4*3 possible states, lets lowball it
	// const int seenMax = 141*141*100;
	bool seenM[141][141][3][3][3] = {0};

	set_t *seen = set_new(141*141);
	
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
	pqueue_push(&q, 0, 0, 0, 1, 0, 0);

	// consume less heated points
	for(point_t *p = pqueue_pop(&q); p != NULL; p = pqueue_pop(&q)){
		
		if(set_exists(seen, p, ((void*)(&p->travel + 1) - (void*)&p->x)) != seenM[p->x][p->y][p->dirx+1][p->diry+1][p->travel])
			set_exists(seen, p, ((void*)(&p->travel + 1) - (void*)&p->x));

		if(seenM[p->x][p->y][p->dirx+1][p->diry+1][p->travel])
			continue;

		seenM[p->x][p->y][p->dirx+1][p->diry+1][p->travel] = true;
		set_add(seen, p, ((void*)(&p->travel + 1) - (void*)&p->x));

		// if(set_exists(seen, p, ((void*)(&p->travel + 1) - (void*)&p->x)))
		// 	continue;

		// set_add(seen, p, ((void*)(&p->travel + 1) - (void*)&p->x));

		visited[p->y][p->x][0] = p->y - p->diry;
		visited[p->y][p->x][1] = p->x - p->dirx;
		if(p->heat < losses[p->y][p->x])
			losses[p->y][p->x] = p->heat;	

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
			pqueue_push(&q, 
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
			pqueue_push(&q, 
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
			pqueue_push(&q, 
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

	// printf("From:\n");
	// for(int i = 0; i < rails->h; i++){
	// 	for(int j = 0; j < rails->w; j++){
	// 		if(j > 0)
	// 			printf(", ");

	// 		if(visited[i][j][0] == -1 && visited[i][j][1] == -1)
	// 			printf("[%c,%c]", '#', '#');
	// 		else
	// 			printf("[%d,%d]", visited[i][j][0], visited[i][j][1]);
	// 	}
	// 	printf("\n");
	// }

	// int tx = rails->w - 1, ty = rails->h - 1;
	// int i = 0;
	// for(int x = tx, y = ty; x > -1 && y > -1; tx = visited[y][x][0], ty = visited[y][x][1], x = tx, y = ty){
	// 	rails->path[y][x] = '#';
	// 	i++;

	// 	if(i > 1000)
	// 		break;
	// }

	// printf("Path:\n");
	// printStringMatrix(rails->path, rails->w, rails->h);

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
	// too high 1291
	
	// printf("Part 2: %lu\n", );

	free(rails); 
	string_destroy(data);
	return 0;
}