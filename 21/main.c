#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <regex.h>
#include <math.h>
#include "src/worker.h"
#include "src/string+.h"
#include "src/number.h"
#include "src/hash.h"
#include "src/data.h"
#include "src/flood_fill.h"

typedef struct{
	int x;
	int y;
	int d;
	int dx;
	int dy;
}point_t;

typedef struct{
	matrix_t *map;
	point_t start; 
}puzzle_t;

// parse file
puzzle_t *parseInput(const string *data){
	puzzle_t *p = malloc(sizeof(puzzle_t));
	p->map = matrix_from_string(data);

	for(size_t i = 0; i < p->map->h; i++){
		for(size_t j = 0; j < p->map->w; j++){
			if(p->map->rows[i][j] == 'S'){
				p->start.x = j;
				p->start.y = i;
			}
		}
	}
	
	return p;
}

void point_push(queue_t *q, int x, int y, int dx, int dy, int d){
	point_t *p = malloc(sizeof(point_t));
	p->x = x;
	p->y = y;
	p->d = d;
	p->dx = dx;
	p->dy = dy;
	queue_push(q, p);
}

void point_pop(queue_t *q, point_t *p){
	point_t *pop = queue_pop(q);
	*p = *pop;
	free(pop);
}

void printMap(const matrix_t *m){
	for(size_t i = 0; i < m->h; i++){
		for(size_t j = 0; j < m->w; j++){
			switch(m->rows[i][j]){
				case 0:
					printf(".");
				break;

				case -1:
					printf("#");
				break;

				default:
					printf("O");
				break;
			}
		}
		printf("\n");
	}
}

void pace(matrix_t *m, int startx, int starty, int maxDistance){
	queue_t *q = queue_new(true);
	
	m->rows[starty][startx] = 1;
	point_push(q, startx, starty - 1, 0, -1, 1); // up
	point_push(q, startx, starty + 1, 0, 1, 1); // down
	point_push(q, startx + 1, starty, 1, 0, 1); // right
	point_push(q, startx - 1, starty, -1, 0, 1); // left

	point_t n;
	while(q->size > 0){
		point_pop(q, &n);

		if(m->rows[n.y][n.x] == 0){
			if(
				n.d == 0 ||
				maxDistance % 2 == n.d % 2
			){
				m->rows[n.y][n.x] = 1;
				// printMap(m);
			}

			if(n.d < maxDistance){
				// left
				if(n.dx != 1 && n.x > 0)
					point_push(q, n.x - 1, n.y, -1, 0, n.d + 1);

				// right
				if(n.dx != -1 && (size_t)n.x < (m->w - 1))
					point_push(q, n.x + 1, n.y, 1, 0, n.d + 1);

				// up
				if(n.dy != 1 && n.y > 0)
					point_push(q, n.x, n.y - 1, 0, -1, n.d + 1);

				// down
				if(n.dy != -1 && (size_t)n.y < (m->h - 1))
					point_push(q, n.x, n.y + 1, 0, 1, n.d + 1);
			}
		}
	}

	queue_destroy(q);
}

uint64_t pacesFloodLike(const matrix_t *m, int sx, int sy, int max){
	matrix_t *copy = matrix_copy(m);
	for(size_t i = 0; i < copy->h; i++){
		for(size_t j = 0; j < copy->w; j++){
			switch(copy->rows[i][j]){
				case '#':
					copy->rows[i][j] = -1;
				break;
				
				default:
					copy->rows[i][j] = 0;
				break;
			}
		}
	}

	pace(copy, sx, sy, max);

	uint64_t acc = 0;
	for(size_t i = 0; i < copy->h; i++){
		for(size_t j = 0; j < copy->w; j++){
			switch(copy->rows[i][j]){
				case 1:
					acc++;
				break;
			}
		}
	}
	
	// printMap(copy);
	matrix_destroy(copy);
	return acc;
}

uint64_t part1(puzzle_t *p){
	return pacesFloodLike(p->map, p->start.x, p->start.y, 64);
}

bool isSurrounded(const matrix_t *m, int i, int j){
	return 
		(i > 0 && m->rows[i-1][j] == '#') &&
		(i < (int)m->h - 1 && m->rows[i+1][j] == '#') &&
		(j > 0 && m->rows[i][j-1] == '#') &&
		(j < (int)m->w - 1 && m->rows[i][j+1] == '#');
}

uint64_t pacesSimple(const matrix_t *m, int sx, int sy, int max){
	uint64_t acc = 0;
	const uint64_t maxEven = max % 2;
	int d;
	
	for(size_t i = 0; i < m->h; i++){
		for(size_t j = 0; j < m->w; j++){
			switch(m->rows[i][j]){
				case 'S':
				case '.':
					d = labs(sx - (int)j) + labs(sy - (int)i);
					if(d <= max && d % 2 == maxEven && !isSurrounded(m, i, j))
						acc++;
				break;
			}
		}
	}
	
	return acc;
}

uint64_t part1m2(puzzle_t *p){
	return pacesSimple(p->map, p->start.x, p->start.y, 64);
}

uint64_t part2(puzzle_t *p){
	uint64_t const max = 26501365;
	uint64_t gridWidth = max / p->map->w - 1;
	uint64_t odd = (gridWidth / 2) * 2 + 1;
	odd *= odd;
	uint64_t even = ((gridWidth + 1)/ 2) * 2;
	even *= even;

	return
		odd  * pacesSimple(p->map, p->start.x, p->start.y, p->map->w * 2 + 1) + 				// points on full odd grids
		even * pacesSimple(p->map, p->start.x, p->start.y, p->map->w * 2) + 					// points on full even grids
		pacesSimple(p->map, 	p->start.x, 	p->map->h - 1,	p->map->w - 1) + 				// top corner
		pacesSimple(p->map, 	p->start.x,		0, 				p->map->w - 1) + 				// bottom corner
		pacesSimple(p->map, 	p->map->w - 1,	p->start.y, 	p->map->w - 1) + 				// left corner
		pacesSimple(p->map, 	0,				p->start.y, 	p->map->w - 1) + 				// right corner
		(gridWidth + 1) * (																		// outer corners
			pacesSimple(p->map, 	0,				p->map->w - 1, 	(p->map->w / 2) - 1) + 		// tr outer slope
			pacesSimple(p->map, 	0,				0, 				(p->map->w / 2) - 1) + 		// br outer slope
			pacesSimple(p->map, 	p->map->w - 1,  p->map->w - 1, 	(p->map->w / 2) - 1) + 		// tl outer slope
			pacesSimple(p->map, 	p->map->w - 1,	0, 				(p->map->w / 2) - 1)    	// bl outer slope
		) +
		(gridWidth) * (																			// inner corners
			pacesSimple(p->map, 	0,				p->map->w - 1, 	(p->map->w * 3 / 2) - 1) + 	// tr inner slope
			pacesSimple(p->map, 	0,				0, 				(p->map->w * 3 / 2) - 1) + 	// br inner slope
			pacesSimple(p->map, 	p->map->w - 1,  p->map->w - 1, 	(p->map->w * 3 / 2) - 1) + 	// tl inner slope
			pacesSimple(p->map, 	p->map->w - 1,	0, 				(p->map->w * 3 / 2) - 1)    // bl inner slope
		);
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

	puzzle_t *p = parseInput(data);
	
	printf("Part 1: %lu\n", part1(p));
	printf("Part 1m2: %lu\n", part1m2(p));
	printf("Part 2: %lu\n", part2(p));

	string_destroy(data);
	return 0;
}