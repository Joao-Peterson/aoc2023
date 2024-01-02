#include "flood_fill.h"
#include "data.h"

typedef struct{
	size_t x;
	size_t y;
}fill_pos_t;

void fill_pos_push(queue_t *q, size_t x, size_t y){
	fill_pos_t *new = malloc(sizeof(fill_pos_t));
	new->y = y;
	new->x = x;
	queue_push(q, new); 
}

void fill_pos_pop(queue_t *q, fill_pos_t *pos){
	fill_pos_t *got = queue_pop(q);
	*pos = *got;
	free(got);
}

void flood_fill_int(int **matrix, size_t height, size_t width, size_t startx, size_t starty, int empty, int fill){
	queue_t *q = queue_new(true);
	
	fill_pos_push(q, startx, starty);

	fill_pos_t n;
	while(q->size > 0){
		fill_pos_pop(q, &n);

		if(matrix[n.y][n.x] == empty){
			matrix[n.y][n.x] = fill;

			// left
			if(n.x > 0)
				fill_pos_push(q, n.x - 1, n.y);

			// right
			if(n.x < (width - 1))
				fill_pos_push(q, n.x + 1, n.y);

			// up
			if(n.y > 0)
				fill_pos_push(q, n.x, n.y - 1);

			// down
			if(n.y < (height - 1))
				fill_pos_push(q, n.x, n.y + 1);
		}
	}

	queue_destroy(q);
}