#ifndef _FLOOD_FILL_HEADER_
#define _FLOOD_FILL_HEADER_

#include <stdint.h>
#include <string.h>
#include "data.h"

/**
 * @brief flood fill and int matrix using queue based recursion fill
 * @param matrix: an allocated int matrix
 * @param height: the height of the matrix
 * @param width: the width of the matrix
 * @param startx: x starting position
 * @param starty: y starting position
 * @param empty: int value that is considered empty in the matrix
 * @param fill: int value to be used as fill
*/
void flood_fill_int(
	int **matrix, 
	size_t height, size_t width, 
	size_t startx, size_t starty, 
	int empty, int fill
);

/**
 * @brief flood fill and int matrix using queue based recursion fill
 * @param matrix: a matrix_t type, created with matrix_new, matrix_from_string,... 
 * @param startx: x starting position
 * @param starty: y starting position
 * @param empty: int value that is considered empty in the matrix
 * @param fill: int value to be used as fill
*/
void flood_fill_matrix(matrix_t *m, size_t startx, size_t starty, int empty, int fill);

/**
 * @brief flood fill and int matrix using queue based recursion fill
 * @param matrix: a matrix_t type, created with matrix_new, matrix_from_string,... 
 * @param startx: x starting position
 * @param starty: y starting position
 * @param ignore: int value that is ignored and not processed
 * @param empty: int value that is considered empty in the matrix
 * @param maxDistance: max travel distance from the starting point
*/
void flood_fill_matrix_distance(matrix_t *m, size_t startx, size_t starty, int ignore, int empty, int maxDistance);

#endif