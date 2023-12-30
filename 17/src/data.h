#ifndef _DATA_HEADER_
#define _DATA_HEADER_

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

typedef struct node_t node_t;
struct node_t{
	void* value;
	node_t *next;
};

typedef uint32_t(*hashFunction)(const uint8_t *data, size_t size, size_t max);

typedef struct{
	node_t **values;
	uint32_t size;
	bool onws;
	hashFunction hash;
}hashtable_chained_t;

typedef struct{
	hashtable_chained_t *table;
	uint32_t *array;
	uint16_t *tablePos;
	size_t size;
	size_t pos;
}set_t;

uint32_t defaultHashTableHashFunction(const uint8_t *data, size_t size, size_t max);

hashtable_chained_t *hashtable_chained_new(size_t size, bool takeOwnership, hashFunction f);

void hashtable_chained_destroy(hashtable_chained_t *ht);

uint32_t hashtable_chained_set(hashtable_chained_t *h, uint32_t hash, void *value);

uint32_t hashtable_chained_set_value(hashtable_chained_t *h, void *key, size_t keySize, void *value);

node_t *hashtable_chained_get(hashtable_chained_t *h, uint32_t hash);

node_t *hashtable_chained_get_value(hashtable_chained_t *h, void *key, size_t keySize);

set_t *set_new(size_t size);

void set_destroy(set_t *s);

void set_add(set_t *set, void *value, size_t size);

void *set_get(set_t *set, size_t pos);

bool set_exists(set_t *set, void *value, size_t size);

#endif