#include "data.h"
#include "hash.h"

// ------------------------------------------------------------ Linked list --------------------------------------------------------

list_t *list_new(bool takeOwnership){
	list_t *l = calloc(1, sizeof(list_t));
	l->onws = takeOwnership;
	return l;
}

void list_destroy(list_t *l){
	if(l->size > 0){
		node_t *cursor = l->first;
		node_t *t;
		while(cursor != NULL){
			t = cursor->next;

			if(l->onws)
				free(cursor->value);
			
			free(cursor);
			cursor = t;
		}
	}

	free(l);
}

void list_push(list_t *l, void *value){
	node_t *new = calloc(1, sizeof(node_t));
	new->value = value;
	
	if(l->size > 0){
		l->last->next = new;
		new->prev = l->last;
		l->last = new;
	}
	else{
		l->first = new;
		l->last = new;
	}

	l->size++;
}

void list_push_unique(list_t *l, void *value, valueCmpFunction cmpFunc){
	for(node_t *node = l->last; node != NULL; node = node->prev){
		if(cmpFunc(value, node->value) == priority_reject)
			return;
	}

	list_push(l, value);
}

void list_push_priority(list_t *l, void *value, valueCmpFunction cmpFunc){
	node_t *new = calloc(1, sizeof(node_t));
	new->value = value;
	
	if(l->size > 0){
		node_t *cursor;
		for(cursor = l->last; cursor != NULL; cursor = cursor->prev){
			priority_t cmp = cmpFunc(new->value, cursor->value);

			if(cmp == priority_reject){
				if(l->onws)
					free(value);
				free(new);
				return;
			}

			if(cmp == priority_left){
				if(cursor == l->last){
					l->last = new;
					cursor->next = new;
					new->prev = cursor;
				}
				else{
					cursor->next->prev = new;
					new->next = cursor->next;
					cursor->next = new;
					new->prev = cursor;
				}

				break;
			}
		}

		if(cursor == NULL){
			l->first->prev = new;
			new->next = l->first;
			l->first = new;
		}
	}
	else{
		l->first = new;
		l->last = new;
	}

	l->size++;
}

void *list_queue_pop(list_t *l){
	if(l == NULL || l->size == 0) return NULL;

	void *ret = NULL;

	if(l->size == 1){
		ret = l->first->value;
		free(l->first);
		l->first = NULL;
		l->last = NULL;
	}
	else{
		ret = l->first->value;
		l->first = l->first->next;
		free(l->first->prev);
		l->first->prev = NULL;
	}
	
	l->size--;
	return ret;
}

void *list_stack_pop(list_t *l){
	void *ret = NULL;

	if(l->size > 1){
		ret = l->last->value;
		l->last = l->last->prev;
		free(l->last->next);
		l->last->next = NULL;
	}
	else if(l->size == 1){
		ret = l->last->value;
		free(l->last);
		l->first = NULL;
		l->last = NULL;
	}
	
	l->size--;
	return ret;
}

// ------------------------------------------------------------ Dynamic Array ------------------------------------------------------

#define MIN_ARRAY_BLOCK_ALLOC 100

array_t *array_new(void){
	array_t *a = calloc(1, sizeof(array_t));
	a->onws = true;
	a->allocated = MIN_ARRAY_BLOCK_ALLOC;
	a->blockSize = MIN_ARRAY_BLOCK_ALLOC;
	a->size = 0;
	a->raw = calloc(a->allocated, sizeof(void*));
	return a;
}

array_t *array_new_wconf(size_t blockAllocSize, bool takeOwnership){
	array_t *a = calloc(1, sizeof(array_t));
	a->onws = takeOwnership;
	a->allocated = blockAllocSize;
	a->blockSize = blockAllocSize;
	a->size = 0;
	a->raw = calloc(a->allocated, sizeof(void*));
	return a;
}

void array_destroy(array_t *a){
	if(a->onws){
		for(size_t i = 0; i < a->size; i++){
			free(a->raw[i]);	
		}
	}

	free(a->raw);
	free(a);
}

void array_set(array_t *a, size_t pos, void *value){
	if(pos >= a->allocated){
		a->raw = realloc(a->raw, pos + a->blockSize);
		a->size = pos + 1;
	}

	if(a->raw[pos] != NULL && a->onws)
		free(a->raw[pos]);

	a->raw[pos] = value;

	if(pos >= a->size) a->size = pos + 1;
}

size_t array_add(array_t *a, void *value){
	size_t pos = a->size;
	array_set(a, pos, value);
	return pos;
}

size_t array_len(const array_t *a){
	return a->size;
}

void *array_get(const array_t *a, size_t pos){
	if(a == NULL || pos >= a->allocated) return NULL;
	return a->raw[pos];
}

// ------------------------------------------------------------ Queue --------------------------------------------------------------

queue_t *queue_new(bool takeOwnership){
	return list_new(takeOwnership);
}

void queue_destroy(queue_t *q){
	list_destroy(q);
}

void queue_push(queue_t *q, void *value){
	node_t *new = calloc(1, sizeof(node_t));
	new->value = value;
	
	if(q->size > 0){
		q->last->next = new;
		new->prev = q->last;
		q->last = new;
	}
	else{
		q->first = new;
		q->last = new;
	}

	q->size++;
}

void *queue_pop(queue_t *q){
	if(q == NULL || q->size == 0) return NULL;

	void *ret = NULL;

	if(q->size == 1){
		ret = q->first->value;
		free(q->first);
		q->first = NULL;
		q->last = NULL;
	}
	else{
		ret = q->first->value;
		q->first = q->first->next;
		free(q->first->prev);
		q->first->prev = NULL;
	}
	
	q->size--;
	return ret;
}

// ------------------------------------------------------------ Priority Queue -----------------------------------------------------

pqueue_t *pqueue_new(valueCmpFunction cmpFunc, bool takeOwnership){
	pqueue_t *pq = calloc(1, sizeof(pqueue_t));
	pq->onws = takeOwnership;
	pq->cmpFunc = cmpFunc;
	return pq;
}

void pqueue_destroy(pqueue_t *pq){
	list_destroy((list_t*)pq);
}

void pqueue_push(pqueue_t *pq, void *value){
	node_t *new = calloc(1, sizeof(node_t));
	new->value = value;
	
	if(pq->size > 0){
		node_t *cursor;
		for(cursor = pq->last; cursor != NULL; cursor = cursor->prev){
			priority_t cmp = pq->cmpFunc(new->value, cursor->value);

			if(cmp == priority_reject){
				if(pq->onws)
					free(value);
				free(new);
				return;
			}

			if(cmp == priority_left){
				if(cursor == pq->last){
					pq->last = new;
					cursor->next = new;
					new->prev = cursor;
				}
				else{
					cursor->next->prev = new;
					new->next = cursor->next;
					cursor->next = new;
					new->prev = cursor;
				}

				break;
			}
		}

		if(cursor == NULL){
			pq->first->prev = new;
			new->next = pq->first;
			pq->first = new;
		}
	}
	else{
		pq->first = new;
		pq->last = new;
	}

	pq->size++;
}

void *pqueue_pop(pqueue_t *pq){
	return queue_pop((queue_t*)pq);
}

// ------------------------------------------------------------ Stack --------------------------------------------------------------

stack_t *stack_new(bool takeOwnership){
	return list_new(takeOwnership);
}

void stack_destroy(stack_t *q){
	list_destroy(q);
}

void stack_push(stack_t *q, void *value){
	queue_push(q, value);
}

void *stack_pop(stack_t *q){
	void *ret = NULL;

	if(q->size > 1){
		ret = q->last->value;
		q->last = q->last->prev;
		free(q->last->next);
		q->last->next = NULL;
	}
	else if(q->size == 1){
		ret = q->last->value;
		free(q->last);
		q->first = NULL;
		q->last = NULL;
	}
	
	q->size--;
	return ret;
}

// ------------------------------------------------------------ Hash table ---------------------------------------------------------

uint32_t defaultHashTableHashFunction(const uint8_t *data, size_t size, size_t max){
	return djb2_hash(data, size) % max;
}

hashtable_t *hashtable_new(size_t size, bool takeOwnership, hashFunction f){
	hashtable_t *h = malloc(sizeof(hashtable_t));
	h->size = size;
	h->onws = takeOwnership;
	h->hash = f != NULL ? f : defaultHashTableHashFunction;
	h->values = calloc(h->size, sizeof(node_t*));
	return h;
}

void hashtable_destroy(hashtable_t *h){
	for(size_t i = 0; i < h->size; i++){
		if(h->values[i] != NULL){
			node_t *cursor = h->values[i];
			while(cursor != NULL){
				node_t *t = cursor->next;

				if(h->onws)
					free(cursor->value);

				free(cursor);
				cursor = t;
			}
		}
	}

	free(h);
}

uint32_t hashtable_set_by_hash(hashtable_t *h, uint32_t hash, void *value){
	uint32_t pos = 0;
	
	if(h->values[hash] == NULL){
		h->values[hash] = malloc(sizeof(node_t));
		h->values[hash]->value = value;
		h->values[hash]->next = NULL;
	}
	else{
		node_t *cursor = h->values[hash];

		while(cursor->next != NULL){
			pos++;
			cursor = cursor->next;
		}

		cursor->next = malloc(sizeof(node_t));
		cursor->next->value = value;
		cursor->next->next = NULL;
	}

	return pos;
}

uint32_t hashtable_set_bin(hashtable_t *h, void *key, size_t keySize, void *value){
	uint32_t hash = h->hash(key, keySize, h->size);
	return hashtable_set_by_hash(h, hash, value);
}

uint32_t hashtable_set(hashtable_t *h, char *key, void *value){
	return hashtable_set_bin(h, key, strlen(key), value);
}

node_t *hashtable_get_by_hash(hashtable_t *h, uint32_t hash){
	return h->values[hash];
}

node_t *hashtable_get_bin(hashtable_t *h, void *key, size_t keySize){
	uint32_t hash = h->hash(key, keySize, h->size);
	return hashtable_get_by_hash(h, hash);
}

node_t *hashtable_get(hashtable_t *h, char *key){
	return hashtable_get_bin(h, key, strlen(key));
}

// ------------------------------------------------------------ Set ----------------------------------------------------------------

set_t *set_new(size_t size){
	set_t *s = malloc(sizeof(set_t));
	s->size = size;
	s->pos = 0;
	s->table = hashtable_new(2*size, true, NULL);
	s->array = malloc(sizeof(uint32_t) * size);
	s->tablePos = malloc(sizeof(uint16_t) * size);
	return s;
}

void set_destroy(set_t *s){
	free(s->array);
	free(s->tablePos);
	hashtable_destroy(s->table);
}

int set_add(set_t *set, void *value, size_t size){
	if(set->pos + 1 >= set->size) return -1;

	uint32_t hash = set->table->hash(value, size, set->table->size);
	uint32_t pos = hashtable_set_by_hash(set->table, hash, value);
	set->array[set->pos] = hash;
	set->tablePos[set->pos] = pos;
	set->pos++;
	return set->pos;
}

node_t *set_get(set_t *set, size_t pos){
	if(pos > set->pos) return NULL;

	uint32_t hash = set->array[pos];
	node_t *value = hashtable_get_by_hash(set->table, hash);

	if(value == NULL || value->next == NULL)
		return value;

	node_t *cursor = value;
	uint16_t tablePos = set->tablePos[pos];
	uint16_t i = 0;
	while(cursor != NULL){
		if(i == tablePos)
			return cursor->value;

		i++;
		cursor = cursor->next;
	}

	return NULL;
}

bool set_exists(set_t *set, void *value, size_t size){
	node_t *nodes = hashtable_get_bin(set->table, value, size);

	if(nodes == NULL)
		return false;

	node_t *cursor = nodes;
	while(cursor != NULL){
		if(!memcmp(value, cursor->value, size))
			return true;

		cursor = cursor->next;
	}

	return false;
}

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
	string_ite_t ite = string_split(lines, "\n");

	size_t h = 1;
	for(char *i = lines->raw; *i != '\0'; i++){
		if(*i == '\n')
			h++;
	}

	matrix_t *m = malloc(sizeof(matrix_t));
	m->rows = malloc(sizeof(int*) * h);

	size_t l = 0;
	for(string *line = string_next(&ite); line != NULL; line = string_next(&ite)){
		if(l == 0)
			m->w = line->len;

		m->rows[l] = malloc(sizeof(int) * m->w);
		for(size_t c = 0; c < m->w; c++)
			m->rows[l][c] = line->raw[c];
		
		l++;
		string_destroy(line);
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
	string *s = string_new_sized(m->h * m->w * 5 + m->w * m->h + m->h);
	for(size_t i = 0; i < m->h; i++){
		for(size_t j = 0; j < m->w; j++){
			if(j > 0 && colSep)
				string_write(s, "%c", 2, colSep);

			if(intWidth)
				string_write(s, "%0*d", 15, intWidth, m->rows[i][j]);
			else
				string_write(s, "%d", 15, m->rows[i][j]);
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
