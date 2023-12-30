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

void printMatrix(char **m, size_t w, size_t h){
	for(size_t i = 0; i < h; i++){
		printf("%.*s\n", (int)w, m[i]);
	}
}

void printNumMatrix(char **m, size_t w, size_t h, bool commaSep){
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
	char **losses;
	char **path;
	int w;
	int h;
}rails_t;

typedef struct point_t point_t;
struct point_t{
	int x;
	int y;
	int travel;
	int dirx;
	int diry;
	int heat;
	int fromx;
	int fromy;
	point_t *next;
};

typedef struct{
	point_t *first;
	point_t *last;
	size_t size;
}pqueue_t;

void pqueue_push(pqueue_t *q, int heat, int x, int y, int dirx, int diry, int travel, int fromx, int fromy){
	point_t *p = calloc(1, sizeof(point_t));
	p->y = y;
	p->x = x;
	p->heat = heat;
	p->next = NULL;
	p->dirx = dirx;
	p->diry = diry;
	p->travel = travel;
	p->fromy = fromy;
	p->fromx = fromx;

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

bool bound(int x, int y, int x0, int x1, int y0, int y1){
	return !(x < x0 || x > x1 || y < y0 || y > y1);
}

uint64_t part1(const rails_t *rails){
	int visited[145][145][2] = {0};
	memset(visited, -1, sizeof(int)*145*145*2);
	bool seen[145][145][3][3][3] = {0};

	uint64_t ret = 0;
	pqueue_t q = {0};

	// start tl
	pqueue_push(&q, 0, 0, 0, 0, 0, 0, -1, -1);
	for(point_t *p = pqueue_pop(&q); p != NULL; p = pqueue_pop(&q)){
		// if processed
		if(seen[p->y][p->x][p->diry+1][p->dirx+1][p->travel])
			continue;

		// meta
		seen[p->y][p->x][p->diry+1][p->dirx+1][p->travel] = true;
		visited[p->y][p->x][0] = p->fromx;
		visited[p->y][p->x][1] = p->fromy;
		rails->losses[p->y][p->x] = p->heat;

		// end br
		if(p->x == rails->w - 1 && p->y == rails->h - 1){
			ret = p->heat; 
			break;
		}

		int newx = p->x + p->dirx;
		int newy = p->y + p->diry;
		// has direction and in within the travel limit
		if(
			(p->dirx || p->diry) && 
			p->travel < 3 &&
			bound(newx, newy, 0, rails->w - 1, 0, rails->h - 1)
		){
			pqueue_push(&q, 
				p->heat + rails->heatLoss[newy][newx],
				newx,
				newy,
				p->dirx, 
				p->diry,
				p->travel + 1,
				p->x,
				p->y
			);
		}		

		// adjacent cells
		int dirx;
		int diry;
		
		// move on hor. axis if p dir is not on hor.
		if(!p->dirx){
			// right
			dirx = 1;
			diry = 0;
			newx = p->x + dirx;
			newy = p->y + diry;
			if(bound(newx, newy, 0, rails->w - 1, 0, rails->h - 1)){
				pqueue_push(&q, 
					p->heat + rails->heatLoss[newy][newx],
					newx,
					newy,
					dirx, 
					diry,
					1,
					p->x,
					p->y
				);
			}

			// left
			dirx = -1;
			diry = 0;
			newx = p->x + dirx;
			newy = p->y + diry;
			if(bound(newx, newy, 0, rails->w - 1, 0, rails->h - 1)){
				pqueue_push(&q, 
					p->heat + rails->heatLoss[newy][newx],
					newx,
					newy,
					dirx, 
					diry,
					1,
					p->x,
					p->y
				);
			}
		}

		// move on ver. axis if p is not on ver.
		if(!p->diry){
			// down
			dirx = 0;
			diry = 1;
			newx = p->x + dirx;
			newy = p->y + diry;
			if(bound(newx, newy, 0, rails->w - 1, 0, rails->h - 1)){
				pqueue_push(&q, 
					p->heat + rails->heatLoss[newy][newx],
					newx,
					newy,
					dirx, 
					diry,
					1,
					p->x,
					p->y
				);
			}

			// up
			dirx = 0;
			diry = -1;
			newx = p->x + dirx;
			newy = p->y + diry;
			if(bound(newx, newy, 0, rails->w - 1, 0, rails->h - 1)){
				pqueue_push(&q, 
					p->heat + rails->heatLoss[newy][newx],
					newx,
					newy,
					dirx, 
					diry,
					1,
					p->x,
					p->y
				);
			}
		}

		// int dirs[4][2] = {{0, 1}, {0, -1}, {1, 0}, {-1, 0}};
		// for(size_t i = 0; i < 4; i++){
		// 	if((dirs[i][0] != p->dirx && dirs[i][0] != -p->dirx) || (dirs[i][1] != p->diry && dirs[i][1] != -p->diry)){
		// 		newx = p->x + dirs[i][0];
		// 		newy = p->y + dirs[i][1];
		// 		if(bound(newx, newy, 0, rails->w - 1, 0, rails->h - 1)){
		// 			pqueue_push(&q, 
		// 				p->heat + rails->heatLoss[newy][newx],
		// 				newx,
		// 				newy,
		// 				dirs[i][0], 
		// 				dirs[i][1],
		// 				1,
		// 				p->x,
		// 				p->y
		// 			);
		// 		}
		// 	}
		// }
	}

	printf("Heatloss:\n");
	printNumMatrix(rails->heatLoss, rails->w, rails->h, false);
	printf("Losses:\n");
	printNumMatrix(rails->losses, rails->w, rails->h, true);

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
	// printMatrix(rails->path, rails->w, rails->h);

	return ret;
}

// parse file
void *parseInput(const string *data){

	string_ite_t iterator = string_split(data, "\n");
	rails_t *rails = calloc(1, sizeof(rails_t));
	rails->heatLoss = calloc(150, sizeof(char*));
	rails->losses = calloc(150, sizeof(char*));
	rails->path = calloc(150, sizeof(char*));

	for(string *line = string_next(&iterator); line != NULL; line = string_next(&iterator)){
		rails->w = line->len;

		for(size_t i = 0; i < line->len; i++)
			line->raw[i] -= '0';
			
		rails->losses[rails->h] = calloc(line->len + 1, sizeof(char));
		memset(rails->losses[rails->h], 0, line->len);
		rails->path[rails->h] = calloc(line->len + 1, sizeof(char));
		memset(rails->path[rails->h], ' ', line->len);
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