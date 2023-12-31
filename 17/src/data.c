#include "data.h"
#include "hash.h"

// ------------------------------------------------------------ Linked list --------------------------------------------------------

linkedlist_t *linkedlist_new(bool takeOwnership){
	linkedlist_t *ll = calloc(1, sizeof(linkedlist_t));
	ll->onws = takeOwnership;
	return ll;
}

void linkedlist_destroy(linkedlist_t *ll){
	if(ll->size > 0){
		node_t *cursor = ll->first;
		node_t *t;
		while(cursor != NULL){
			t = cursor->next;

			if(ll->onws)
				free(cursor->value);
			
			free(cursor);
			cursor = t;
		}
	}

	free(ll);
}

// ------------------------------------------------------------ Queue --------------------------------------------------------------

queue_t *queue_new(bool takeOwnership){
	return linkedlist_new(takeOwnership);
}

void queue_destroy(queue_t *q){
	linkedlist_destroy(q);
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
	void *ret = NULL;

	if(q->size > 1){
		ret = q->first->value;
		q->first = q->first->next;
		free(q->first->prev);
		q->first->prev = NULL;
	}
	else if(q->size == 1){
		ret = q->first->value;
		free(q->first);
		q->first = NULL;
		q->last = NULL;
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
	linkedlist_destroy((linkedlist_t*)pq);
}

void pqueue_push(pqueue_t *pq, void *value){
	node_t *new = calloc(1, sizeof(node_t));
	new->value = value;
	
	if(pq->size > 0){
		node_t *cursor;
		for(cursor = pq->last; cursor != NULL; cursor = cursor->prev){
			if(!pq->cmpFunc(new->value, cursor->value)){
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
	return linkedlist_new(takeOwnership);
}

void stack_destroy(stack_t *q){
	linkedlist_destroy(q);
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

// ------------------------------------------------------------ Set ----------------------------------------------------------------

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

int set_add(set_t *set, void *value, size_t size){
	if(set->pos + 1 >= set->size) return -1;

	uint32_t hash = set->table->hash(value, size, set->table->size);
	uint32_t pos = hashtable_chained_set(set->table, hash, value);
	set->array[set->pos] = hash;
	set->tablePos[set->pos] = pos;
	set->pos++;
	return set->pos;
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