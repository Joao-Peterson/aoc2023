#ifndef _DATA_HEADER_
#define _DATA_HEADER_

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

// ------------------------------------------------------------ Types --------------------------------------------------------------

typedef struct node_t node_t;
struct node_t{
	void* value;
	node_t *next;
	node_t *prev;
};

typedef uint32_t(*hashFunction)(const uint8_t *data, size_t size, size_t max);

/**
 * @brief this function should return: 
 * true, if 'a' should be put before of 'b'
 * false, if 'b' should be put before of 'a'.
 * Remember to cast the void* to the correct pointer type of your data 
 * @param a: is always the element to be inserted
 * @param b: is an element already in
*/
typedef bool(*valueCmpFunction)(void *a, void *b);

// ------------------------------------------------------------ Linked list --------------------------------------------------------

typedef struct{
	node_t *first;
	node_t *last;
	bool onws;
	size_t size;
}linkedlist_t;

linkedlist_t *linkedlist_new(bool takeOwnership);

void linkedlist_destroy(linkedlist_t *ll);

// ------------------------------------------------------------ Queue --------------------------------------------------------------

typedef linkedlist_t queue_t;

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

typedef linkedlist_t stack_t;

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
}hashtable_chained_t;

hashtable_chained_t *hashtable_chained_new(size_t size, bool takeOwnership, hashFunction f);

void hashtable_chained_destroy(hashtable_chained_t *ht);

uint32_t hashtable_chained_set(hashtable_chained_t *h, uint32_t hash, void *value);

uint32_t hashtable_chained_set_value(hashtable_chained_t *h, void *key, size_t keySize, void *value);

node_t *hashtable_chained_get(hashtable_chained_t *h, uint32_t hash);

node_t *hashtable_chained_get_value(hashtable_chained_t *h, void *key, size_t keySize);

// ------------------------------------------------------------ Set ----------------------------------------------------------------

typedef struct{
	hashtable_chained_t *table;
	uint32_t *array;
	uint16_t *tablePos;
	size_t size;
	size_t pos;
}set_t;

set_t *set_new(size_t size);

void set_destroy(set_t *s);

int set_add(set_t *set, void *value, size_t size);

void *set_get(set_t *set, size_t pos);

bool set_exists(set_t *set, void *value, size_t size);

#endif