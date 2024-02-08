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
#include "src/matrix.h"
#include "src/ite.h"

typedef struct{
	point_t points[50];
	int pointDistance[50];
	int pointsSize;
}edge_t;

typedef struct{
	point_t p;
	int d;
}travel_point_t;

typedef struct{
	matrix_t *map;
	point_t start;
	point_t end;
	list_t *points;
}puzzle_t;

cmp_t pointCmp(void *pa, void *pb){
	const point_t *a = (point_t*)pa;
	const point_t *b = (point_t*)pb;
	if((a->y == b->y) && (a->x == b->x)) return cmp_reject;
	return cmp_accept;
}

// parse file
puzzle_t *parseInput(const string *data){
	puzzle_t *p = malloc(sizeof(puzzle_t));
	p->map = matrix_from_string(data);
	p->start = (point_t){.y = 0, .x = 1};
	p->end 	 = (point_t){.y = p->map->h - 1, .x = p->map->w - 2};

	// get list of intersections
	p->points = list_new_custom_queue(true, pointCmp);
	list_push(p->points, point_new(p->start.y, p->start.x));
	list_push(p->points, point_new(p->end.y, p->end.x));
	
	for(size_t i = 0; i < p->map->h; i++){
		for(size_t j = 0; j < p->map->w; j++){
			if(p->map->rows[i][j] == '#') continue;
			int neighbors = 0;

			if(matrix_inside(p->map, i-1, j) && p->map->rows[i-1][j] != '#')
				neighbors++;
			if(matrix_inside(p->map, i+1, j) && p->map->rows[i+1][j] != '#')
				neighbors++;
			if(matrix_inside(p->map, i, j-1) && p->map->rows[i][j-1] != '#')
				neighbors++;
			if(matrix_inside(p->map, i, j+1) && p->map->rows[i][j+1] != '#')
				neighbors++;
			
			if(neighbors >= 3){
				list_push(p->points, point_new(i, j));
			}
		}
	}

	return p;
}

int dict_set_point(dict_t *d, int y, int x){
	point_t p = (point_t){.y = y, .x = x};
	return dict_add_bin(d, &p, sizeof(point_t), (void*)1);
}

void dict_unset_point(dict_t *d, int y, int x){
	point_t p = (point_t){.y = y, .x = x};
	dict_remove_bin(d, &p, sizeof(point_t));
}

bool dict_check_point(const dict_t *d, int y, int x){
	point_t p = (point_t){.y = y, .x = x};
	return dict_exists_bin(d, &p, sizeof(point_t));
}

int64_t dfs(const puzzle_t *p, point_t point, dict_t *graph, dict_t *seen){
	if(point.y == p->end.y && point.x == p->end.x)
		return 0;

	int64_t d = -1;

	list_t *l = dict_get_bin(graph, &point, sizeof(point_t));
	list_ite li = list_iterate(l);
	dict_set_point(seen, point.y, point.x);
	foreach(travel_point_t *, tp, li){
		if(dict_check_point(seen, tp->p.y, tp->p.x)) continue;

		int64_t dd = dfs(p, tp->p, graph, seen);
		if(dd > -1){
			dd += tp->d;
			if(dd > d) d = dd;
		}
	}
	dict_unset_point(seen, point.y, point.x);

	return d;
}

uint64_t part1(const puzzle_t *p){	
	dict_t *graph = dict_new(100);

	list_ite ite = list_iterate(p->points);
	// for every vertix on intersections, count distance to other edges
	foreach(point_t*, point, ite){

		list_t *stack = list_new_custom_stack(false, pointCmp);
		dict_t *seen = dict_new(141);

		// starting point
		travel_point_t *init = malloc(sizeof(travel_point_t));
		*init = (travel_point_t){.p = *point, .d = 0};
		dict_set_point(seen, point->y, point->x);
		list_push(stack, init);

		// navigate through path 
		list_ite pops = list_popall(stack);
		foreach(travel_point_t*, t, pops){
			// if reached an intersection vertix
			if(t->d > 0 && list_exists(p->points, &(t->p))){
				if(!dict_exists_bin(graph, point, sizeof(point_t))){
					list_t *points = list_new_custom_queue(true, NULL);
					dict_add_bin(graph, point, sizeof(point_t), points);
				}

				list_t *points = dict_get_bin(graph, point, sizeof(point_t));
				list_push(points, t);
				continue;
			}

			int ny, nx;
			// step right
			if(p->map->rows[t->p.y][t->p.x] == '>' || p->map->rows[t->p.y][t->p.x] == '.'){
				ny = t->p.y;
				nx = t->p.x + 1;
				if(matrix_inside(p->map, ny, nx) && p->map->rows[ny][nx] != '#' && !dict_check_point(seen, ny, nx)){
					dict_set_point(seen, ny, nx);
					travel_point_t *nt = malloc(sizeof(travel_point_t));
					*nt = (travel_point_t){.p = (point_t){.y = ny, .x = nx}, .d = t->d + 1};
					list_push(stack, nt);
				}
			}
			// step left
			if(p->map->rows[t->p.y][t->p.x] == '<' || p->map->rows[t->p.y][t->p.x] == '.'){
				ny = t->p.y;
				nx = t->p.x - 1;
				if(matrix_inside(p->map, ny, nx) && p->map->rows[ny][nx] != '#' && !dict_check_point(seen, ny, nx)){
					dict_set_point(seen, ny, nx);
					travel_point_t *nt = malloc(sizeof(travel_point_t));
					*nt = (travel_point_t){.p = (point_t){.y = ny, .x = nx}, .d = t->d + 1};
					list_push(stack, nt);
				}
			}
			// step down
			if(p->map->rows[t->p.y][t->p.x] == 'v' || p->map->rows[t->p.y][t->p.x] == '.'){
				ny = t->p.y + 1;
				nx = t->p.x;
				if(matrix_inside(p->map, ny, nx) && p->map->rows[ny][nx] != '#' && !dict_check_point(seen, ny, nx)){
					dict_set_point(seen, ny, nx);
					travel_point_t *nt = malloc(sizeof(travel_point_t));
					*nt = (travel_point_t){.p = (point_t){.y = ny, .x = nx}, .d = t->d + 1};
					list_push(stack, nt);
				}
			}
			// step up
			if(p->map->rows[t->p.y][t->p.x] == '^' || p->map->rows[t->p.y][t->p.x] == '.'){
				ny = t->p.y - 1;
				nx = t->p.x;
				if(matrix_inside(p->map, ny, nx) && p->map->rows[ny][nx] != '#' && !dict_check_point(seen, ny, nx)){
					dict_set_point(seen, ny, nx);
					travel_point_t *nt = malloc(sizeof(travel_point_t));
					*nt = (travel_point_t){.p = (point_t){.y = ny, .x = nx}, .d = t->d + 1};
					list_push(stack, nt);
				}
			}

			free(t);
		}

		list_destroy(stack);
		dict_destroy(seen);
	}

	// printf("Dictionary graph: \n");
	// dict_ite dt = dict_iterate(graph);
	// foreach(key_value_t, kv, dt){
	// 	const point_t *from = kv.key;
	// 	const list_t *l = kv.value;

	// 	printf("(%d,%d): ", (int)from->y, (int)from->x);

	// 	list_ite li = list_iterate(l);
	// 	int i = 0;
	// 	foreach(travel_point_t *, tp, li){
	// 		if(i > 0 )
	// 			printf(", ");
	// 		printf("{(%d,%d): %d}", (int)tp->p.y, (int)tp->p.x, tp->d);

	// 		i++;
	// 	}
	// 	printf("\n");
	// }

	dict_t *seenDfs = dict_new(141);
	int64_t res = dfs(p, p->start, graph, seenDfs);
	dict_destroy(seenDfs);
	dict_destroy(graph);
	return res;
}

uint64_t part2(const puzzle_t *p){	
	dict_t *graph = dict_new(100);

	list_ite ite = list_iterate(p->points);
	// for every vertix on intersections, count distance to other edges
	foreach(point_t*, point, ite){

		list_t *stack = list_new_custom_stack(false, pointCmp);
		dict_t *seen = dict_new(141);

		// starting point
		travel_point_t *init = malloc(sizeof(travel_point_t));
		*init = (travel_point_t){.p = *point, .d = 0};
		dict_set_point(seen, point->y, point->x);
		list_push(stack, init);

		// navigate through path 
		list_ite pops = list_popall(stack);
		foreach(travel_point_t*, t, pops){
			// if reached an intersection vertix
			if(t->d > 0 && list_exists(p->points, &(t->p))){
				if(!dict_exists_bin(graph, point, sizeof(point_t))){
					list_t *points = list_new_custom_queue(true, NULL);
					dict_add_bin(graph, point, sizeof(point_t), points);
				}

				list_t *points = dict_get_bin(graph, point, sizeof(point_t));
				list_push(points, t);
				continue;
			}

			if(p->map->rows[t->p.y][t->p.x] != '#'){
				int ny, nx;
				
				// step right
				ny = t->p.y;
				nx = t->p.x + 1;
				if(matrix_inside(p->map, ny, nx) && p->map->rows[ny][nx] != '#' && !dict_check_point(seen, ny, nx)){
					dict_set_point(seen, ny, nx);
					travel_point_t *nt = malloc(sizeof(travel_point_t));
					*nt = (travel_point_t){.p = (point_t){.y = ny, .x = nx}, .d = t->d + 1};
					list_push(stack, nt);
				}
				// step left
				ny = t->p.y;
				nx = t->p.x - 1;
				if(matrix_inside(p->map, ny, nx) && p->map->rows[ny][nx] != '#' && !dict_check_point(seen, ny, nx)){
					dict_set_point(seen, ny, nx);
					travel_point_t *nt = malloc(sizeof(travel_point_t));
					*nt = (travel_point_t){.p = (point_t){.y = ny, .x = nx}, .d = t->d + 1};
					list_push(stack, nt);
				}
				// step down
				ny = t->p.y + 1;
				nx = t->p.x;
				if(matrix_inside(p->map, ny, nx) && p->map->rows[ny][nx] != '#' && !dict_check_point(seen, ny, nx)){
					dict_set_point(seen, ny, nx);
					travel_point_t *nt = malloc(sizeof(travel_point_t));
					*nt = (travel_point_t){.p = (point_t){.y = ny, .x = nx}, .d = t->d + 1};
					list_push(stack, nt);
				}
				// step up
				ny = t->p.y - 1;
				nx = t->p.x;
				if(matrix_inside(p->map, ny, nx) && p->map->rows[ny][nx] != '#' && !dict_check_point(seen, ny, nx)){
					dict_set_point(seen, ny, nx);
					travel_point_t *nt = malloc(sizeof(travel_point_t));
					*nt = (travel_point_t){.p = (point_t){.y = ny, .x = nx}, .d = t->d + 1};
					list_push(stack, nt);
				}
			}
			
			free(t);
		}

		list_destroy(stack);
		dict_destroy(seen);
	}

	dict_t *seenDfs = dict_new(141);
	int64_t res = dfs(p, p->start, graph, seenDfs);
	dict_destroy(seenDfs);
	dict_destroy(graph);
	return res;
}

int main(int argc, char**argv){
	string *data;
	
	if(argc > 1)
		data = string_from_filename(argv[1], NULL);	// from arg filename
	else
		data = string_from_file(stdin, NULL);      	// from pipe

	puzzle_t *p = parseInput(data);

	printf("Part 1: %lu\n", part1(p));
	printf("Part 2: %lu\n", part2(p));

	string_destroy(data);
	return 0;
}