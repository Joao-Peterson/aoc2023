#include "linalg.h"
#include <math.h>

dmatrix_t *dmatrix_gaussian_elimination(const dmatrix_t *A, const dmatrix_t *b){
	if(A->h != A->w || b->h != A->w) return NULL;
	
	dmatrix_t *x = dmatrix_copy(b);

	for(size_t k = 0; (k + 1) < A->h; k++){
		// partial pivoting
		double w = fabs(A->rows[k][k]);
		size_t r = k;
		for(size_t i = (k + 1); i < A->h; i++) {
			if(fabs(A->rows[i][k]) > w) {
				w = fabs(A->rows[i][k]);
				r = i;
			}
		}
		
		// swap rows
		if (r != k) {
			double temp;
			for(size_t i = k; i < A->h; i++) {
				temp = A->rows[k][i];
				A->rows[k][i] = A->rows[r][i];
				A->rows[r][i] = temp;
			}
			temp = x->rows[k][0];
			x->rows[k][0] = x->rows[r][0];
			x->rows[r][0] = temp;
		}

		// forward elimination
		for(size_t i = (k + 1); i < A->h; i++){
			double m = A->rows[i][k] / A->rows[k][k];
			for(size_t j = 0; j < A->h; j++)
				A->rows[i][j] -= m * A->rows[k][j];

			x->rows[i][0] -= m * x->rows[k][0];
		}

		for(int i = 0; i < 6; i++){
			for(int j = 0; j < 6; j++)
				printf("%16ld, ", (int64_t)A->rows[i][j]);
			printf("\n");
		}		
		printf("\n");
	}

	// back substitution
	for(int64_t i = (A->h - 1); i >= 0; i--){
		for(size_t j = (i + 1); j < A->h; j++)
			x->rows[i][0] -= A->rows[i][j] * x->rows[j][0];

		x->rows[i][0] /= A->rows[i][i];
	}

	for(int i = 0; i < 6; i++){
		printf("%16ld\n", (int64_t)x->rows[i][0]);
	}		
	printf("\n");

	return x;	
}

dmatrix_t *dmatrix_gauss_sidel(const dmatrix_t *A, const dmatrix_t *b, int iterations){
	if(A->w != A->h || b->w != A->w) return NULL;
	dmatrix_t *x = dmatrix_new(A->w, 1, 0);

	// iterations
	for(int n = 0; n < iterations; n++){
		// row / var
		for(size_t i = 0; i < A->h; i++){
			
			double a = 0;

			for(size_t j = 0; j < A->w; j++){
				if(j == i) continue;
				a += A->rows[i][j] * x->rows[0][j];
			}

			x->rows[0][i] = (b->rows[0][i] - a) / A->rows[i][i];
		}

		printf("Iteration %d: ", n);
		for(size_t i = 0; i < A->h; i++)
			printf("%.4G ", x->rows[0][i]);
		printf("\n");
	}

	return x;
}