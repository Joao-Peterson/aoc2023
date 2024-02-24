#ifndef _LINALG_HEADER_
#define _LINALG_HEADER_

#include "matrix.h"

dmatrix_t *dmatrix_gaussian_elimination(const dmatrix_t *A, const dmatrix_t *b);

dmatrix_t *dmatrix_gauss_sidel(const dmatrix_t *A, const dmatrix_t *b, int iterations);

#endif