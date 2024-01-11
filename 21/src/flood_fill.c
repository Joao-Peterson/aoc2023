#include "flood_fill.h"

typedef struct{
	size_t x;
	size_t y;
	int d;
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

void flood_fill_matrix(matrix_t *m, size_t startx, size_t starty, int empty, int fill){
	queue_t *q = queue_new(true);
	
	fill_pos_push(q, startx, starty);

	fill_pos_t n;
	while(q->size > 0){
		fill_pos_pop(q, &n);

		if(m->rows[n.y][n.x] == empty){
			m->rows[n.y][n.x] = fill;

			// left
			if(n.x > 0)
				fill_pos_push(q, n.x - 1, n.y);

			// right
			if(n.x < (m->w - 1))
				fill_pos_push(q, n.x + 1, n.y);

			// up
			if(n.y > 0)
				fill_pos_push(q, n.x, n.y - 1);

			// down
			if(n.y < (m->h - 1))
				fill_pos_push(q, n.x, n.y + 1);
		}
	}

	queue_destroy(q);
}

void fill_pos_push_d(queue_t *q, size_t x, size_t y, size_t d){
	fill_pos_t *new = malloc(sizeof(fill_pos_t));
	new->y = y;
	new->x = x;
	new->d = d;
	queue_push(q, new); 
}

void flood_fill_matrix_distance(matrix_t *m, size_t startx, size_t starty, int ignore, int empty, int maxDistance){
	queue_t *q = queue_new(true);
	
	fill_pos_push_d(q, startx, starty, 0);

	fill_pos_t n;
	while(q->size > 0){
		fill_pos_pop(q, &n);

		if(
			n.d <= maxDistance &&
			m->rows[n.y][n.x] != ignore && 
			(m->rows[n.y][n.x] == empty || n.d < m->rows[n.y][n.x])
		){
			m->rows[n.y][n.x] = n.d;

			// left
			if(n.x > 0)
				fill_pos_push_d(q, n.x - 1, n.y, n.d + 1);

			// right
			if(n.x < (m->w - 1))
				fill_pos_push_d(q, n.x + 1, n.y, n.d + 1);

			// up
			if(n.y > 0)
				fill_pos_push_d(q, n.x, n.y - 1, n.d + 1);

			// down
			if(n.y < (m->h - 1))
				fill_pos_push_d(q, n.x, n.y + 1, n.d + 1);
		}
	}

	queue_destroy(q);
}