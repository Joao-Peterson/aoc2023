#include "matrix.h"

// ------------------------------------------------------------ Matrix -------------------------------------------------------------

matrix_t *matrix_new(size_t w, size_t h, int init){
	if(!w && !h) return NULL;

	matrix_t *m = malloc(sizeof(matrix_t));
	m->w = w;
	m->h = h;
	m->rows = malloc(sizeof(int*) * h);
	
	for(size_t i = 0; i < h; i++){
		m->rows[i] = malloc(sizeof(int) * w);
		for(size_t j = 0; j < w; j++){
			m->rows[i][j] = init;
		}
	}

	return m;
}

matrix_t *matrix_from_string(const string *lines){
	// compute how many lines
	size_t h = 1;
	for(char *i = lines->raw; *i != '\0'; i++){
		if(*i == '\n')
			h++;
	}

	matrix_t *m = malloc(sizeof(matrix_t));
	m->rows = malloc(sizeof(int*) * h);

	size_t l = 0;
	string_ite ite = string_split(lines, "\n");
	for(string line = ite.next(&ite); ite.yield; line = ite.next(&ite)){
		if(l == 0)
			m->w = line.len;

		m->rows[l] = malloc(sizeof(int) * m->w);
		for(size_t c = 0; c < m->w; c++)
			m->rows[l][c] = line.raw[c];
		
		l++;
	}

	m->h = l;

	return m;
}

matrix_t *matrix_from_file(FILE *file, size_t *read){
	string *f = string_from_file(file, read);
	if(f == NULL) return NULL;

	matrix_t *m = matrix_from_string(f);
	string_destroy(f);
	return m;
}

matrix_t *matrix_from_filename(const char *filename, size_t *read){
	string *f = string_from_filename(filename, read);
	if(f == NULL) return NULL;

	matrix_t *m = matrix_from_string(f);
	string_destroy(f);
	return m;
}

matrix_t *matrix_copy(const matrix_t *m){
	matrix_t *new = matrix_new(m->w, m->h, 0);

	for(size_t i = 0; i < m->h; i++)
		memcpy(new->rows[i], m->rows[i], m->w * sizeof(int));

	return new;
}

void matrix_destroy(matrix_t *m){
	for(size_t i = 0; i < m->h; i++)
		free(m->rows[i]);

	free(m->rows);
	free(m);
}

bool matrix_inside(const matrix_t *m, size_t y, size_t x){
	return y < m->h && x < m->w;
}

bool matrix_pinside(const matrix_t *m, point_t p){
	return matrix_inside(m, p.y, p.x);
}

size_t matrix_index_point(const matrix_t *m, size_t y, size_t x){
	if(!matrix_inside(m, y, x)) return 0;
	return y * m->w + x;
}

size_t matrix_index_p(const matrix_t *m, point_t p){
	return matrix_index_point(m, p.y, p.x);
}

string *matrix_print_char(const matrix_t *m, char colSep, char rowSep){
	// 5 is average len of the string representantion of INT32_MAX
	// + m->h * m->w * 5 for numbers
	// + m->h * m->w for commas
	// + m->h for line endings
	string *s = string_new_sized(m->h * m->w * 5 + m->w * m->h + m->h);
	for(size_t i = 0; i < m->h; i++){
		for(size_t j = 0; j < m->w; j++){
			if(j > 0 && colSep)
				string_write(s, "%c", 2, colSep);

			string_write(s, "%c", 2, (char)m->rows[i][j]);
		}

		if(rowSep)
		string_write(s, "%c", 2, rowSep);
	}
	return s;
}

string *matrix_print_int(const matrix_t *m, int intWidth, char colSep, char rowSep){
	// 5 is average len of the string representantion of INT32_MAX
	// + m->h * m->w * 5 for numbers
	// + m->h * m->w for commas
	// + m->h for line endings
	int max = 1;
	for(int w = 0; w < intWidth; w++)
		max *= 10;

	// increment to always have one more with to compensate the extra space of 'printf("% *d")' 
	intWidth++;
	
	string *s = string_new_sized(m->h * m->w * 5 + m->w * m->h + m->h);
	for(size_t i = 0; i < m->h; i++){
		for(size_t j = 0; j < m->w; j++){
			if(j > 0 && colSep)
				string_write(s, "%c", 2, colSep);

			if(intWidth){
				int toPrint = 0;
				if(m->rows[i][j] < 0)
					// for negative numbers its the opposite
					toPrint = m->rows[i][j] <= -max ? -max + 1 : m->rows[i][j];
				else
					// if has width, print min of n digits of the number, which if bigger than the maximum of digits, will be truncated as max - 1
					// So width 3 is a max of 1000, the number 5000 is >= 1000 so it will be 1000 - 1 = 999
					toPrint = m->rows[i][j] >= max ? max - 1 : m->rows[i][j];
					
				string_write(s, "% *d", 15, intWidth, toPrint);
			}
			else{
				string_write(s, "% d", 15, m->rows[i][j]);
			}
		}

		if(rowSep)
		string_write(s, "%c", 2, rowSep);
	}
	return s;
}

string *matrix_print_csv(const matrix_t *m){
	return matrix_print_int(m, 0, ',', '\n');
}

string *matrix_print_trunc(const matrix_t *m, unsigned int truncate){
	return matrix_print_int(m, truncate, ' ', '\n');
}

point_t *point_new(size_t y, size_t x){
	point_t *p = malloc(sizeof(point_t));
	p->y = y;
	p->x = x;
	return p;
}

void point_destroy(point_t *p){
	free(p);
}

// ------------------------------------------------------------ Double matrix ------------------------------------------------------

// !trivial
dmatrix_t *dmatrix_new(size_t w, size_t h, double init){
	if(w < 1 || h < 1) return NULL;
	
	dmatrix_t *m = malloc(sizeof(dmatrix_t));
	m->rows = malloc(sizeof(double*) * h);
	for(size_t i = 0; i < h; i++)
		m->rows[i] = malloc(sizeof(double) * w);

	m->h = h;
	m->w = w;

	for(size_t i = 0; i < h; i++)
		for(size_t j = 0; j < w; j++)
			m->rows[i][j] = init;

	return m;
}

dmatrix_t *dmatrix_new_identity(size_t w, size_t h){
	dmatrix_t *m = dmatrix_new(w, h, 0);

	size_t min = w < h ? w : h;
	for(size_t i = 0; i < min; i++)
		m->rows[i][i] = 1;

	return m;
}

// !trivial
dmatrix_t *dmatrix_from_string(const string *lines){
	// compute how many lines
	size_t h = 1;
	for(char *i = lines->raw; *i != '\0'; i++){
		if(*i == '\n')
			h++;
	}

	dmatrix_t *m = malloc(sizeof(dmatrix_t));
	m->rows = malloc(sizeof(double*) * h);

	size_t l = 0;
	string_ite ite = string_split(lines, "\n");
	for(string line = ite.next(&ite); ite.yield; line = ite.next(&ite)){
		if(l == 0)
			m->w = line.len;

		m->rows[l] = malloc(sizeof(double) * m->w);
		for(size_t c = 0; c < m->w; c++)
			m->rows[l][c] = line.raw[c];
		
		l++;
	}

	m->h = l;

	return m;
}

dmatrix_t *dmatrix_from_file(FILE *file, size_t *read){
	string *f = string_from_file(file, read);
	if(f == NULL) return NULL;
	dmatrix_t *m = dmatrix_from_string(f);
	string_destroy(f);
	return m;	
}

dmatrix_t *dmatrix_from_filename(const char *filename, size_t *read){
	string *f = string_from_filename(filename, read);
	if(f == NULL) return NULL;
	dmatrix_t *m = dmatrix_from_string(f);
	string_destroy(f);
	return m;
}

dmatrix_t *dmatrix_copy(const dmatrix_t *m){
	dmatrix_t *new = dmatrix_new(m->w, m->h, 0);

	for(size_t i = 0; i < m->h; i++)
		memcpy(new->rows[i], m->rows[i], m->w * sizeof(double));

	return new;
}

void dmatrix_destroy(dmatrix_t *m){
	for(size_t i = 0; i < m->h; i++)
		free(m->rows[i]);

	free(m->rows);
	free(m);
}
