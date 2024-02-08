#include "data.h"
#include "hash.h"
#include "ite.h"

node_t *node_clone(const node_t *node){
	node_t *new = calloc(1, sizeof(node_t));
	new->value = node->value;
	return new;
}

// ------------------------------------------------------------ Linked list --------------------------------------------------------

// !trivial
list_t *list_new(){
	list_t *l = calloc(1, sizeof(list_t));
	l->type = list_type_queue;
	return l;
}

list_t *list_new_queue(){
	return list_new();
}

list_t *list_new_stack(){
	list_t *l = list_new();
	l->type = list_type_stack;
	return l;
}

list_t *list_new_custom_queue(bool takeOwnership, cmpFunc priorityCmp){
	list_t *l = list_new_queue();
	l->onws = takeOwnership;
	l->cmpFunc = priorityCmp;
	return l;
}

list_t *list_new_custom_stack(bool takeOwnership, cmpFunc priorityCmp){
	list_t *l = list_new_stack();
	l->onws = takeOwnership;
	l->cmpFunc = priorityCmp;
	return l;
}

// !trivial
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

// !trivial
void list_push(list_t *l, void *value){
	if(l->size > 0){
		node_t *cursor = l->last;

		// insert at the end normally
		if(l->cmpFunc == NULL) goto insert;

		// insert comparing from right to left
		for(cursor = l->last; cursor != NULL; cursor = cursor->prev){
			// comparison
			switch(l->cmpFunc(cursor->value, value)){
				case cmp_left:
					continue;
				case cmp_right:
					goto insert;
				case cmp_reject:
					return;
				default:
				case cmp_accept:
					goto insert;
			}
		}

		// insert node
		insert:
		{
			node_t *new = calloc(1, sizeof(node_t));
			new->value = value;
			
			// insert as first
			if(cursor == NULL){
				l->first->prev = new;
				new->next = l->first;
				l->first = new;
			}
			// insert as last
			else if(cursor == l->last){
				l->last = new;
				cursor->next = new;
				new->prev = cursor;
			}
			// insert in between
			else{
				cursor->next->prev = new;
				new->next = cursor->next;
				cursor->next = new;
				new->prev = cursor;
			}
		}
	}
	else{
		node_t *new = calloc(1, sizeof(node_t));
		new->value = value;
		l->first = new;
		l->last = new;
	}

	l->size++;
}

// !trivial
void list_merge(list_t *dest, list_t *consumed){
	if(consumed == NULL || consumed->first == NULL || dest->onws != consumed->onws) return;
	if(dest->size == 0){
		dest->first = consumed->first;
		dest->last = consumed->last;
	}
	else{
		consumed->first->prev = dest->last;
		dest->last->next = consumed->first;
		dest->last = consumed->last;
	}

	dest->size += consumed->size;
	list_destroy(consumed);
}

// !trivial
void *list_pop(list_t *l){
	if(l == NULL || l->size == 0) return NULL;
	void *ret = NULL;

	switch(l->type){
		case list_type_stack:
			if(l->size == 1){
				ret = l->last->value;
				free(l->last);
				l->first = NULL;
				l->last = NULL;
			}
			else{
				ret = l->last->value;
				l->last = l->last->prev;
				free(l->last->next);
				l->last->next = NULL;
			}
		break;

		default:
		case list_type_queue:
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
		break;
	}
	
	l->size--;
	return ret;
}

// !trivial
bool list_exists(const list_t *l, void *value){
	if(l->cmpFunc == NULL) return true;
	list_ite ite = list_iterate(l);
	foreach(void*, v, ite){
		if(l->cmpFunc(v, value) == cmp_reject)
			return true;
	}
	return false;
}

size_t list_size(const list_t *l){
	return l->size;
}

void *list_next(list_ite *i){
	if(i->state == NULL){
		i->yield = false;
		return NULL;
	}

	void *v = i->state->value;
	i->state = i->l->type == list_type_stack ? i->state->prev : i->state->next;
	return v;
}

list_ite list_iterate(const list_t *l){
	return (list_ite){
		.next = list_next,
		.yield = true,
		.l = (list_t*)l,
		.state = l->type == list_type_stack ? l->last : l->first
	};
}

void *list_popnext(list_ite *i){
	void *v = list_pop(i->l);
	
	if(v == NULL)
		i->yield = false;
		
	return v;
}

list_ite list_popall(list_t *l){
	return (list_ite){
		.next = list_popnext,
		.yield = true,
		.state = NULL,
		.l = l
	};
}

// ------------------------------------------------------------ Dynamic Array ------------------------------------------------------

// !trivial
array_t *array_new_custom(bool takeOwnership, size_t blockSize){
	array_t *a = calloc(1, sizeof(array_t));
	a->onws = takeOwnership;
	a->allocated = blockSize;
	a->blockSize = blockSize;
	a->size = 0;
	a->raw = calloc(sizeof(void*), a->allocated);
	return a;
}

array_t *array_new(void){
	return array_new_custom(false, MIN_ARRAY_BLOCK_SIZE);
}

// !trivial
void array_destroy(array_t *a){
	if(a->onws){
		for(size_t i = 0; i < a->size; i++){
			free(a->raw[i]);	
		}
	}

	free(a->raw);
	free(a);
}

// !trivial
/**
 * If the position is already has a value then it's overwritten. 
 * If overwritten and the array has ownership, then the old value is deallocated.
 * If the position is bigger than the the current allocated size, the array is expanded by the 'blockSize'. 
 * If the position is bigger than the the current array size, the array size is expanded, this leaves empty values behind! 
*/
bool array_set_raw(array_t *a, size_t pos, void *value){
	if(pos >= a->allocated){
		size_t prev = a->allocated;
		a->allocated = pos + a->blockSize;
		a->raw = realloc(a->raw, a->allocated * sizeof(void*));
		memset(a->raw + prev, 0, (a->allocated - prev) * sizeof(void*));
	}

	if(a->raw[pos] != NULL && a->onws)
		free(a->raw[pos]);

	a->raw[pos] = value;
	if(pos >= a->size) a->size = pos + 1;

	return true;
}

bool array_set(array_t *a, size_t pos, void *value){
	// if(pos >= a->size) return false;
	return array_set_raw(a, pos, value);
}

size_t array_add(array_t *a, void *value){
	size_t pos = a->size;
	array_set_raw(a, pos, value);
	return pos;
}

// !trivial
void *array_get_raw(array_t *a, size_t pos, bool remove, bool rearrange){
	if(a == NULL || pos >= a->allocated) return NULL;
	void *val = a->raw[pos];
	if(remove){
		a->raw[pos] = NULL;

		if(rearrange && a->size > 0){
			a->size -= 1;
			size_t i = pos;
			while(pos < a->size){
				a->raw[i] = a->raw[i+1];
				i++;
			}
		}
	} 

	return val;
}

void *array_get(const array_t *a, size_t pos){
	return array_get_raw((array_t*)a, pos, false, false);
}

void *array_remove(array_t *a, size_t pos){
	return array_get_raw(a, pos, true, false);
}

void *array_pop(array_t *a, size_t pos){
	return array_get_raw(a, pos, true, true);
}

size_t array_size(const array_t *a){
	return a->size;
}

// ------------------------------------------------------------ Hash table ---------------------------------------------------------

uint32_t defaultHashTableHashFunction(const uint8_t *data, size_t size, size_t max){
	return djb2_hash(data, size) % max;
}

// !trivial
hashtable_t *hashtable_new_custom(size_t size, bool takeOwnership, hashFunction f){
	hashtable_t *h = malloc(sizeof(hashtable_t));
	h->size = size;
	h->onws = takeOwnership;
	h->hash = f != NULL ? f : defaultHashTableHashFunction;
	h->values = calloc(h->size, sizeof(kv_node_t*));
	h->collisions = 0;
	return h;
}

hashtable_t *hashtable_new(size_t size){
	return hashtable_new_custom(size, false, defaultHashTableHashFunction);
}

// !trivial
void hashtable_destroy(hashtable_t *h){
	for(size_t i = 0; i < h->size; i++){
		if(h->values[i] != NULL){
			kv_node_t *cursor = h->values[i];
			while(cursor != NULL){
				kv_node_t *t = cursor->next;

				if(h->onws)
					free(cursor->value);

				free(cursor->key);
				free(cursor);
				cursor = t;
			}
		}
	}

	free(h);
}

// !trivial
uint32_t hashtable_set_by_hash(hashtable_t *h, uint32_t hash, const void *key, size_t keySize, void *value){
	kv_node_t *cursor = h->values[hash];
	if(cursor == NULL){
		h->values[hash] = malloc(sizeof(kv_node_t));
		cursor = h->values[hash];
		cursor->next = NULL;
		cursor->prev = NULL;
	}
	else{
		h->collisions++;
		while(cursor->next != NULL){
			// if already in, return hash
			if(cursor->keySize == keySize && !memcmp(cursor->key, key, keySize))
				return hash;
				
			cursor = cursor->next;
		}

		cursor->next = malloc(sizeof(kv_node_t));
		cursor->next->prev = cursor;
		cursor->next->next = NULL;
		cursor = cursor->next;
	}

	cursor->key = malloc(keySize);
	memcpy(cursor->key, key, keySize);
	cursor->keySize = keySize;
	cursor->value = value;

	return hash;
}

uint32_t hashtable_set_bin(hashtable_t *h, void *key, size_t keySize, void *value){
	uint32_t hash = h->hash(key, keySize, h->size);
	return hashtable_set_by_hash(h, hash, key, keySize, value);
}

uint32_t hashtable_set(hashtable_t *h, char *key, void *value){
	return hashtable_set_bin(h, key, strlen(key), value);
}

// !trivial
kv_node_t *hashtable_get_by_hash(hashtable_t *h, uint32_t hash, const void *key, size_t keySize, bool remove){
	kv_node_t *node = h->values[hash];

	while(node != NULL){
		if(node->keySize == keySize && !memcmp(node->key, key, keySize)){
			if(remove){
				if(node->prev != NULL)
					node->prev->next = node->next;
				if(node->next != NULL)
					node->next->prev = node->prev;
				if(node == h->values[hash])
					h->values[hash] = node->next;
			}

			break;			
		}
		node = node->next;
	}

	return node;
}

void *hashtable_get_raw(hashtable_t *h, void *key, size_t keySize, bool remove){
	uint32_t hash = h->hash(key, keySize, h->size);
	kv_node_t *kv = hashtable_get_by_hash(h, hash, key, keySize, remove);

	void *value = kv->value;

	if(remove){
		free(kv->key);
		free(kv);
	}

	return value;
}

void *hashtable_remove_bin(hashtable_t *h, void *key, size_t keySize){
	return hashtable_get_raw(h, key, keySize, true);
}

void *hashtable_remove(hashtable_t *h, char *key){
	return hashtable_remove_bin(h, key, strlen(key));
}

void *hashtable_get_bin(hashtable_t *h, void *key, size_t keySize){
	return hashtable_get_raw(h, key, keySize, false);
}

void *hashtable_get(hashtable_t *h, char *key){
	return hashtable_get_bin(h, key, strlen(key));
}

bool hashtable_exists_bin(hashtable_t *h, void *key, size_t keySize){
	return hashtable_get_bin(h, key, keySize) == NULL ? false : true;
}

bool hashtable_exists(hashtable_t *h, char *key){
	return hashtable_get_bin(h, key, strlen(key)) == NULL ? false : true;
}

// ------------------------------------------------------------ Dictionary ---------------------------------------------------------

// !trivial
dict_t *dict_new_custom(size_t size, bool takeOwnership){
	dict_t *d = malloc(sizeof(dict_t));
	d->table = hashtable_new_custom(size, takeOwnership, NULL);
	d->array = array_new_custom(true, size);
	return d;
}

dict_t *dict_new(size_t size){
	return dict_new_custom(size, false);
}

// !trivial
void dict_destroy(dict_t *d){
	hashtable_destroy(d->table);
	array_destroy(d->array);
	free(d);
}

// !trivial
int64_t dict_set_bin(dict_t *d, size_t pos, void *key, size_t keySize, void *value){
	if(pos >= d->array->size) return -1;
	
	uint32_t hash = d->table->hash(key, keySize, d->table->size);

	// if not exists
	if(hashtable_get_by_hash(d->table, hash, key, keySize, false) != NULL) return -1;

	// array
	key_value_t *a = malloc(sizeof(key_value_t));
	a->value = value;
	a->key = malloc(keySize);
	memcpy(a->key, key, keySize);
	a->keySize = keySize;
	array_set(d->array, pos, a);

	// hashtable
	size_t *s = malloc(sizeof(size_t));
	*s = pos;
	hashtable_set_by_hash(d->table, hash, key, keySize, s);

	return (int64_t)pos;
}

int64_t dict_set(dict_t *d, size_t pos, char *key, void *value){
	return dict_set_bin(d, pos, key, strlen(key), value);
}

int64_t dict_add_bin(dict_t *d, void *key, size_t keySize, void *value){
	d->array->size++;
	return dict_set_bin(d, d->array->size - 1, key, keySize, value);
}

int64_t dict_add(dict_t *d, char *key, void *value){
	return dict_add_bin(d, key, strlen(key), value);
}

// !trivial
void *dict_get_bin_raw(dict_t *d, void *key, size_t keySize, bool remove){
	uint32_t hash = d->table->hash(key, keySize, d->table->size);

	// get table entry
	kv_node_t *kv = hashtable_get_by_hash(d->table, hash, key, keySize, remove);

	// if not exists
	if(kv == NULL) return NULL;

	// value
	const size_t *pos = kv->value;

	// get key value pair
	key_value_t *a = array_get_raw(d->array, *pos, remove, true);

	void *value = a->value;

	if(remove){
		free(kv->key);
		free(kv->value);
		free(kv);
		free(a->key);
		free(a);
	}

	return value;
}

// !trivial
void *dict_get_at_raw(dict_t *d, size_t pos, bool remove){
	key_value_t *a = array_get_raw(d->array, pos, remove, true);
	if(a == NULL) return NULL;
	void *value = a->value;
	
	if(remove){
		free(a->key);
		free(a);
	}

	return value;
}

void *dict_get_bin(const dict_t *d, void *key, size_t keySize){
	return dict_get_bin_raw((dict_t*)d, key, keySize, false);
}

void *dict_get(const dict_t *d, char *key){
	return dict_get_bin_raw((dict_t*)d, key, strlen(key), false);
}

void *dict_get_at(const dict_t *d, size_t pos){
	return dict_get_at_raw((dict_t*)d, pos, false);
}

void *dict_remove_bin(dict_t *d, void *key, size_t keySize){
	return dict_get_bin_raw(d, key, keySize, true);
}

void *dict_remove(dict_t *d, char *key){
	return dict_get_bin_raw(d, key, strlen(key), true);
}

void *dict_remove_at(dict_t *d, size_t pos){
	return dict_get_at_raw(d, pos, true);
}

bool dict_exists_bin(const dict_t *d, void *key, size_t keySize){
	return dict_get_bin_raw((dict_t*)d, key, keySize, false) != NULL;
}

bool dict_exists(const dict_t *d, char *key){
	return dict_get_bin_raw((dict_t*)d, key, strlen(key), false) != NULL;
}

bool dict_exists_at(const dict_t *d, size_t pos){
	return dict_get_at_raw((dict_t*)d, pos, false) != NULL;
}

// !trivial
int64_t dict_pos_of_bin(const dict_t *d, void *key, size_t keySize){
	size_t *pos = hashtable_get_bin(d->table, key, keySize);
	return (pos == NULL) ? -1 : (int64_t)*pos;
}

int64_t dict_pos_of(const dict_t *d, char *key){
	return dict_pos_of_bin(d, key, strlen(key));
}

// !trivial
key_value_t dict_next(dict_ite *i){
	if(i->pos >= i->d->array->size){
		i->yield = false;
		return (key_value_t){0};
	}

	const key_value_t *a = array_get(i->d->array, i->pos);
	i->pos++;

	if(a == NULL){
		i->yield = false;
		return (key_value_t){0};
	}

	return *a;
}

dict_ite dict_iterate(dict_t *d){
	return (dict_ite){
		.next = dict_next,
		.d = d,
		.pos = 0,
		.yield = true,
	};
}

