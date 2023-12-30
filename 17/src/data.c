#include "data.h"
#include "hash.h"

uint32_t defaultHashTableHashFunction(const uint8_t *data, size_t size, size_t max){
	return djb2_hash(data, size) % max;
}

hashtable_chained_t *hashtable_chained_new(size_t size, bool takeOwnership, hashFunction f){
	hashtable_chained_t *h = malloc(sizeof(hashtable_chained_t));
	h->size = size;
	h->onws = takeOwnership;
	h->hash = f != NULL ? f : defaultHashTableHashFunction;
	h->values = calloc(h->size, sizeof(node_t*));
	return h;
}

void hashtable_chained_destroy(hashtable_chained_t *h){
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

uint32_t hashtable_chained_set(hashtable_chained_t *h, uint32_t hash, void *value){
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

uint32_t hashtable_chained_set_value(hashtable_chained_t *h, void *key, size_t keySize, void *value){
	uint32_t hash = h->hash(key, keySize, h->size);
	return hashtable_chained_set(h, hash, value);
}

node_t *hashtable_chained_get(hashtable_chained_t *h, uint32_t hash){
	return h->values[hash];
}

node_t *hashtable_chained_get_value(hashtable_chained_t *h, void *key, size_t keySize){
	uint32_t hash = h->hash(key, keySize, h->size);
	return hashtable_chained_get(h, hash);
}

set_t *set_new(size_t size){
	set_t *s = malloc(sizeof(set_t));
	s->size = size;
	s->pos = 0;
	s->table = hashtable_chained_new(2*size, true, NULL);
	s->array = malloc(sizeof(uint32_t) * size);
	s->tablePos = malloc(sizeof(uint16_t) * size);
	return s;
}

void set_destroy(set_t *s){
	free(s->array);
	free(s->tablePos);
	hashtable_chained_destroy(s->table);
}

void set_add(set_t *set, void *value, size_t size){
	if(set->pos + 1 >= set->size) return;

	uint32_t hash = set->table->hash(value, size, set->table->size);
	uint32_t pos = hashtable_chained_set(set->table, hash, value);
	set->array[set->pos] = hash;
	set->tablePos[set->pos] = pos;
	set->pos++;
}

void *set_get(set_t *set, size_t pos){
	if(pos > set->pos) return NULL;

	uint32_t hash = set->array[pos];
	node_t *value = hashtable_chained_get(set->table, hash);

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
	node_t *nodes = hashtable_chained_get_value(set->table, value, size);

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