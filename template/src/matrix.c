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
