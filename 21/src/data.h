#ifndef _DATA_HEADER_
#define _DATA_HEADER_

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "string+.h"

// ------------------------------------------------------------ Types --------------------------------------------------------------

typedef struct node_t node_t;
struct node_t{
	void* value;
	node_t *next;
	node_t *prev;
};

typedef uint32_t(*hashFunction)(const uint8_t *data, size_t size, size_t max);

typedef enum{
	priority_reject = 0,
	priority_accept = 1,
	priority_left,
	priority_right,
}priority_t;

/**
 * @brief this function should return: 
 * priority_right, if 'a' should be put before of 'b'
 * priority_left, if 'a' should be put after of 'b'
 * priority_reject, if 'a' shall not be inserted
 * Remember that lists, queues and stacks grow to the left
 * Remember to cast the void* to the correct pointer type of your data
 * @param a: is always the element to be inserted
 * @param b: is an element already in
*/
typedef priority_t(*valueCmpFunction)(void *a, void *b);

// ------------------------------------------------------------ Linked list --------------------------------------------------------

typedef struct{
	node_t *first;
	node_t *last;
	bool onws;
	size_t size;
}list_t;

list_t *list_new(bool takeOwnership);

void list_destroy(list_t *l);

void list_push(list_t *l, void *value);

void list_push_unique(list_t *l, void *value, valueCmpFunction cmpFunc);

void list_push_priority(list_t *l, void *value, valueCmpFunction cmpFunc);

void *list_queue_pop(list_t *l);

void *list_stack_pop(list_t *l);

// ------------------------------------------------------------ Dynamic Array ------------------------------------------------------

typedef struct{
	void **raw;
	size_t blockSize;
	size_t allocated;
	size_t size;
	bool onws;
}array_t;

array_t *array_new();

array_t *array_new_wconf(size_t blockAllocSize, bool takeOwnership);

void array_destroy(array_t *a);

void array_set(array_t *a, size_t pos, void *value);

size_t array_add(array_t *a, void *value);

size_t array_len(const array_t *a);

void *array_get(const array_t *a, size_t pos);

// ------------------------------------------------------------ Queue --------------------------------------------------------------

typedef list_t queue_t;

queue_t *queue_new(bool takeOwnership);

void queue_destroy(queue_t *q);

void queue_push(queue_t *q, void *value);

void *queue_pop(queue_t *q);

// ------------------------------------------------------------ Priority Queue -----------------------------------------------------

typedef struct{
	node_t *first;
	node_t *last;
	bool onws;
	size_t size;
	valueCmpFunction cmpFunc;
}pqueue_t;

pqueue_t *pqueue_new(valueCmpFunction cmpFunc, bool takeOwnership);

void pqueue_destroy(pqueue_t *pq);

void pqueue_push(pqueue_t *pq, void *value);

void *pqueue_pop(pqueue_t *pq);

// ------------------------------------------------------------ Stack --------------------------------------------------------------

typedef list_t stack_t;

stack_t *stack_new(bool takeOwnership);

void stack_destroy(stack_t *s);

void stack_push(stack_t *q, void *value);

void *stack_pop(stack_t *q);

// ------------------------------------------------------------ Hash table ---------------------------------------------------------

typedef struct{
	node_t **values;
	uint32_t size;
	bool onws;
	hashFunction hash;
}hashtable_t;

hashtable_t *hashtable_new(size_t size, bool takeOwnership, hashFunction f);

void hashtable_destroy(hashtable_t *ht);

uint32_t hashtable_set(hashtable_t *h, char *key, void *value);

node_t *hashtable_get(hashtable_t *h, char *key);

uint32_t hashtable_set_bin(hashtable_t *h, void *key, size_t keySize, void *value);

node_t *hashtable_get_bin(hashtable_t *h, void *key, size_t keySize);

// ------------------------------------------------------------ Set ----------------------------------------------------------------

typedef struct{
	hashtable_t *table;
	uint32_t *array;
	uint16_t *tablePos;
	size_t size;
	size_t pos;
}set_t;

set_t *set_new(size_t size);

void set_destroy(set_t *s);

int set_add(set_t *set, void *value, size_t size);

node_t *set_get(set_t *set, size_t pos);

bool set_exists(set_t *set, void *value, size_t size);

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