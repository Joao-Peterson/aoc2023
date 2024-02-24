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
node_t *list_push(list_t *l, void *value){
	node_t *new;
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
					return NULL;
				default:
				case cmp_accept:
					goto insert;
			}
		}

		// insert node
		insert:
		{
			new = calloc(1, sizeof(node_t));
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
		new = calloc(1, sizeof(node_t));
		new->value = value;
		l->first = new;
		l->last = new;
	}

	l->size++;
	return new;
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
void *list_remove_node(list_t *l, node_t *node){
	if(node->prev != NULL)
		node->prev->next = node->next;
	else
		l->first = node->next;

	if(node->next != NULL)
		node->next->prev = node->prev;
	else
		l->last = node->prev;

	l->size--;
	void *v = node->value;
	free(node);
	return v;
}

void *list_pop(list_t *l){
	if(l == NULL || l->size == 0) return NULL;

	switch(l->type){
		case list_type_stack:
			return list_remove_node(l, l->last);

		default:
		case list_type_queue:
			return list_remove_node(l, l->first);
	}
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

node_t *list_node_at(const list_t *l, size_t pos){
	if(pos >= l->size) return NULL;

	node_t *node = l->first;	
	while(pos-- > 0)
		node = node->next;	

	return node;
}

void *list_at(const list_t *l, size_t pos){
	node_t *n = list_node_at(l, pos);
	if(n == NULL) 
		return NULL;
	else
		return n->value; 
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
			while(i < a->size){
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
	if(kv == NULL) return NULL;
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
	d->h = hashtable_new_custom(size, takeOwnership, NULL);
	d->l = list_new_custom_queue(true, NULL);
	return d;
}

dict_t *dict_new(){
	return dict_new_custom(1024, false);
}

size_t dict_size(const dict_t *d){
	return d->l->size;
}

// !trivial
void dict_destroy(dict_t *d){
	hashtable_destroy(d->h);
	list_destroy(d->l);
	free(d);
}

// !trivial
key_value_t dict_set_bin(dict_t *d, size_t pos, void *key, size_t keySize, void *value){
	// if list has pos
	node_t *node = list_node_at(d->l, pos);
	if(node == NULL) return (key_value_t){0};
	
	// if key isn't already used
	uint32_t hash = d->h->hash(key, keySize, d->h->size);
	const kv_node_t *hentry = hashtable_get_by_hash(d->h, hash, key, keySize, false);
	if(hentry != NULL) return (key_value_t){0};

	// save old
	key_value_t *kv = node->value;
	key_value_t ret = *kv;

	// alter list node
	kv->value = value;
	kv->key = key;
	kv->keySize = keySize;

	// set on hashtable
	hashtable_set_by_hash(d->h, hash, key, keySize, node);

	return ret;
}

key_value_t dict_set(dict_t *d, size_t pos, char *key, void *value){
	return dict_set_bin(d, pos, key, strlen(key), value);
}

// !trivial
int64_t dict_add_bin(dict_t *d, void *key, size_t keySize, void *value){
	uint32_t hash = d->h->hash(key, keySize, d->h->size);
	const kv_node_t *hentry = hashtable_get_by_hash(d->h, hash, key, keySize, false);
	if(hentry != NULL) return -1;
	
	key_value_t *kv = malloc(sizeof(key_value_t));
	kv->key = key;
	kv->keySize = keySize;
	kv->value = value;
	list_push(d->l, kv);
	hashtable_set_by_hash(d->h, hash, key, keySize, d->l->last);
	return d->l->size - 1;
}

int64_t dict_add(dict_t *d, char *key, void *value){
	return dict_add_bin(d, key, strlen(key), value);
}

// !trivial
key_value_t dict_get_bin_raw(dict_t *d, void *key, size_t keySize, bool remove){
	// get table entry, remove from table if necessary
	node_t *node = hashtable_get_raw(d->h, key, keySize, remove);
	if(node == NULL) return (key_value_t){0};

	// value
	key_value_t value = *(key_value_t*)node->value;

	// remove from list
	if(remove)
		list_remove_node(d->l, node);

	return value;
}

// !trivial
key_value_t dict_get_at_raw(dict_t *d, size_t pos, bool remove){
	const node_t *node = list_node_at(d->l, pos);
	if(node == NULL) return (key_value_t){0};

	key_value_t value = *(key_value_t*)node->value;
	
	if(remove){
		hashtable_remove_bin(d->h, value.key, value.keySize);
		void *v = list_remove_node(d->l, (node_t*)node);
		free(v);
	}

	return value;
}

void *dict_get_bin(const dict_t *d, void *key, size_t keySize){
	return dict_get_bin_raw((dict_t*)d, key, keySize, false).value;
}

void *dict_get(const dict_t *d, char *key){
	return dict_get_bin_raw((dict_t*)d, key, strlen(key), false).value;
}

key_value_t dict_get_at(const dict_t *d, size_t pos){
	return dict_get_at_raw((dict_t*)d, pos, false);
}

void *dict_remove_bin(dict_t *d, void *key, size_t keySize){
	return dict_get_bin_raw(d, key, keySize, true).value;
}

void *dict_remove(dict_t *d, char *key){
	return dict_get_bin_raw(d, key, strlen(key), true).value;
}

key_value_t dict_remove_at(dict_t *d, size_t pos){
	return dict_get_at_raw(d, pos, true);
}

bool dict_exists_bin(const dict_t *d, void *key, size_t keySize){
	return dict_get_bin_raw((dict_t*)d, key, keySize, false).key != NULL;
}

bool dict_exists(const dict_t *d, char *key){
	return dict_get_bin_raw((dict_t*)d, key, strlen(key), false).key != NULL;
}

bool dict_exists_at(const dict_t *d, size_t pos){
	return dict_get_at_raw((dict_t*)d, pos, false).key != NULL;
}

// !trivial
key_value_t dict_next(dict_ite *i){
	if(i->node == NULL){
		i->yield = false;
		return (key_value_t){0};
	}

	key_value_t kv = *(key_value_t*)i->node->value;
	i->node = i->node->next;
	return kv;
}

dict_ite dict_iterate(const dict_t *d){
	return (dict_ite){
		.next = dict_next,
		.d = (dict_t*)d,
		.node = d->l->first,
		.yield = true,
	};
}
