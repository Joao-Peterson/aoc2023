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
#include "src/linalg.h"
#include "src/ite.h"

typedef struct{
	double x;
	double y;
	double z;
}vec_t;

typedef struct{
	vec_t p;
	vec_t v;
}trail_t;

typedef struct{
	array_t *trails;
	double min;
	double max;
}puzzle_t;

// parse file
puzzle_t *parseInput(const string *data){
	puzzle_t *p = malloc(sizeof(puzzle_t));
	p->trails = array_new_custom(true, 300);

	string_ite ite = string_split(data, "\n");
	foreach(string, line, ite){
		string_ite lite = string_split(&line, ",@ ");
		string token;
		trail_t *t = malloc(sizeof(trail_t));

		token = lite.next(&lite);
		t->p.x = string_to_double(&token);
		token = lite.next(&lite);
		t->p.y = string_to_double(&token);
		token = lite.next(&lite);
		t->p.z = string_to_double(&token);
		token = lite.next(&lite);
		t->v.x = string_to_double(&token);
		token = lite.next(&lite);
		t->v.y = string_to_double(&token);
		token = lite.next(&lite);
		t->v.z = string_to_double(&token);

		array_add(p->trails, t);
	}
	
	return p;
}

typedef struct{
	double x;
	double y;
	double z;
	double t1;
	double t2;
}vec_time_t;

vec_time_t trailIntersectionXY(const trail_t *t1, const trail_t *t2){
	double x = (t1->v.x*t2->v.x*t1->p.y - t1->v.x*t2->v.x*t2->p.y + t1->v.x*t2->v.y*t2->p.x - t2->v.x*t1->v.y*t1->p.x)/(t1->v.x*t2->v.y - t2->v.x*t1->v.y);
	double y = (t1->v.x*t1->p.y + t1->v.y*x - t1->v.y*t1->p.x)/t1->v.x;
	double dt1 = (y-t1->p.y)/t1->v.y;
	double dt2 = (y-t2->p.y)/t2->v.y;

	return (vec_time_t){
		.x = x,
		.y = y,
		.t1 = dt1,
		.t2 = dt2,
		.z = 0
	};
}

uint64_t part1(puzzle_t *p){
	uint64_t acc = 0;

	for(size_t i = 0; i < p->trails->size; i++){
		const trail_t *t1 = array_get(p->trails, i);

		for(size_t j = i + 1; j < p->trails->size; j++){

			const trail_t *t2 = array_get(p->trails, j);
			vec_time_t o = trailIntersectionXY(t1, t2);
			bool inside = o.x >= p->min && o.x <= p->max && o.y >= p->min && o.y <= p->max;
			bool past = o.t1 < 0 || o.t2 < 0;

			// printf("Hailstone %d: %.3G, %.3G, %.3G @ %.3G, %.3G, %.3G\n", (int)i, t1->p.x, t1->p.y, t1->p.z, t1->v.x, t1->v.y, t1->v.z);
			// printf("Hailstone %d: %.3G, %.3G, %.3G @ %.3G, %.3G, %.3G\n", (int)j, t2->p.x, t2->p.y, t2->p.z, t2->v.x, t2->v.y, t2->v.z);

			// if(isinf(o.x) || isinf(o.y))
			// 	printf("Intersection: (never)\n\n");
			// else
			// 	printf("Intersection: (%.3G, %.3G, %.3G) (%s) (%s) %s\n\n", 
			// 		o.x, o.y, o.z, 
			// 		inside ? "inside" : "outside",
			// 		past ? "past" : "future",
			// 		(!past && inside) ? "VALID!" : ""
			// 	);

			if(inside && !past)
				acc++;
		}
	}

	return acc;
}

typedef struct{
	int64_t A[2][6];
	int64_t b[2];
}linSysTrails_t;

linSysTrails_t linSysTrailsCalc(const trail_t *t1, const trail_t *t2){
	int64_t 
	x1  = t1->p.x,
	y1  = t1->p.y,
	z1  = t1->p.z,
	vx1 = t1->v.x,
	vy1 = t1->v.y,
	vz1 = t1->v.z,
	x2  = t2->p.x,
	y2  = t2->p.y,
	z2  = t2->p.z,
	vx2 = t2->v.x,
	vy2 = t2->v.y,
	vz2 = t2->v.z;
	
	return (linSysTrails_t){
		.A = {
			{vy1 - vy2, -vx1 + vx2,          0, -y1 + y2, x1 - x2,       0},
			{vz1 - vz2,          0, -vx1 + vx2, -z1 + z2,       0, x1 - x2}
		},
		.b = { -vx1*y1 + vx2*y2 + vy1*x1 - vy2*x2, -vx1*z1 + vx2*z2 + vz1*x1 - vz2*x2}
	};
}

uint64_t part2(const puzzle_t *p){
	// int64_t A [6][6];
	// int64_t b [6];

	dmatrix_t *A = dmatrix_new(6, 6, 0);
	dmatrix_t *b = dmatrix_new(1, 6, 0);

	// compute a system with first trail and with the three following ones, adding to the total linear system
	for(int t = 1; t < 4; t++){
		linSysTrails_t sys = linSysTrailsCalc(
			array_get(p->trails, 0), 
			array_get(p->trails, t)
		);

		// copy A
		for(int i = 0; i < 2; i++)
			for(int j = 0; j < 6; j++)
				A->rows[i + (t-1)*2][j] = sys.A[i][j];

		// copy b
		for(int i = 0; i < 2; i++)
			b->rows[i + (t-1)*2][0] = sys.b[i];
	}

	dmatrix_t *x = dmatrix_gaussian_elimination(A, b);

	uint64_t res = (x->rows[0][0] + x->rows[1][0] + x->rows[2][0]);

	dmatrix_destroy(A);
	dmatrix_destroy(b);
	dmatrix_destroy(x);
	return res;
}

int main(int argc, char**argv){
	string *data;
	if(argc > 1) data = string_from_filename(argv[1], NULL);	// from arg filename
	else         data = string_from_file(stdin, NULL);      	// from pipe

	puzzle_t *p = parseInput(data);
	if(!strcmp(argv[1], "test.txt")) p->min = 7, p->max = 27;
	else							 p->min = 200000000000000, p->max = 400000000000000;
	
	printf("Part 1: %lu\n", part1(p));
	printf("Part 2: %lu\n", part2(p));

	string_destroy(data);
	return 0;
}