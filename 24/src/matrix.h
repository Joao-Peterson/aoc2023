#ifndef _MATRIX_HEADER_
#define _MATRIX_HEADER_

#include <string.h>
#include <stdlib.h>
#include "data.h"
#include "string+.h"

// ------------------------------------------------------------ Matrix -------------------------------------------------------------

typedef struct{
	size_t w;
	size_t h;
	int **rows;
}matrix_t;

typedef struct{
	size_t y;
	size_t x;
}point_t;

matrix_t *matrix_new(size_t w, size_t h, int init);

matrix_t *matrix_from_string(const string *lines);

matrix_t *matrix_from_file(FILE *file, size_t *read);

matrix_t *matrix_from_filename(const char *filename, size_t *read);

matrix_t *matrix_copy(const matrix_t *m);

void matrix_destroy(matrix_t *m);

bool matrix_inside(const matrix_t *m, size_t y, size_t x);

bool matrix_pinside(const matrix_t *m, point_t p);

size_t matrix_index_point(const matrix_t *m, size_t y, size_t x);

size_t matrix_index_p(const matrix_t *m, point_t p);

string *matrix_print_char(const matrix_t *m, char colSep, char rowSep);

string *matrix_print_int(const matrix_t *m, int intWidth, char colSep, char rowSep);

string *matrix_print_csv(const matrix_t *m);

string *matrix_print_trunc(const matrix_t *m, unsigned int truncate);

point_t *point_new(size_t y, size_t x);

void point_destroy(point_t *p);

// ------------------------------------------------------------ Double matrix ------------------------------------------------------

typedef struct{
	size_t w;
	size_t h;
	double **rows;
}dmatrix_t;

dmatrix_t *dmatrix_new(size_t w, size_t h, double init);

dmatrix_t *dmatrix_new_identity(size_t w, size_t h);

dmatrix_t *dmatrix_from_string(const string *lines);

dmatrix_t *dmatrix_from_file(FILE *file, size_t *read);

dmatrix_t *dmatrix_from_filename(const char *filename, size_t *read);

dmatrix_t *dmatrix_copy(const dmatrix_t *m);

void dmatrix_destroy(dmatrix_t *m);

// dmatrix_t *dmatrix_sum(dmatrix_t *into, dmatrix_t *src);

#endif