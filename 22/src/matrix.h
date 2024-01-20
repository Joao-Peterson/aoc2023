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

matrix_t *matrix_new(size_t w, size_t h, int init);

matrix_t *matrix_from_string(const string *lines);

matrix_t *matrix_from_file(FILE *file, size_t *read);

matrix_t *matrix_from_filename(const char *filename, size_t *read);

matrix_t *matrix_copy(const matrix_t *m);

void matrix_destroy(matrix_t *m);

string *matrix_print_char(const matrix_t *m, char colSep, char rowSep);

string *matrix_print_int(const matrix_t *m, int intWidth, char colSep, char rowSep);

string *matrix_print_csv(const matrix_t *m);

string *matrix_print_trunc(const matrix_t *m, unsigned int truncate);

#endif