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

typedef struct{
	void *key;
	void* value;
	size_t keySize;
}key_value_t;

typedef enum{
	cmp_reject = 0,
	cmp_accept = 1,
	cmp_left,
	cmp_right,
}cmp_t;

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
typedef cmp_t(*cmpFunc)(void *a, void *b);

// ------------------------------------------------------------ Linked list --------------------------------------------------------

typedef enum{
	list_type_queue,
	list_type_stack
}list_type_t;

typedef struct{
	node_t *first;
	node_t *last;
	bool onws;
	size_t size;
	list_type_t type;
	cmpFunc cmpFunc;
}list_t;

typedef struct list_ite list_ite;
typedef void* (*list_ite_next_func)(list_ite *ite);
struct list_ite{
	list_ite_next_func next;
	bool yield;
	list_t *l;
	node_t *state;
};

/**
 * @brief creates a new list that behaves like a queue.
 * Values in the list are NOT freed when 'list_destroy' is called
*/
list_t *list_new(void);

/**
 * @brief creates a new list that behaves like a queue.
 * Values in the list are NOT freed when 'list_destroy' is called
*/
list_t *list_new_queue(void);

/**
 * @brief creates a new list that behaves like a stack.
 * Values in the list are NOT freed when 'list_destroy' is called
*/
list_t *list_new_stack(void);

/**
 * @brief creates a new list that behaves like a queue
 * @param takeOwnership: true if pushed values should be freed along with the list deallocation 
 * @param priorityCmp: comparison function to determine the priority of the value to be inserted.
 * Should cast and compare 'a' (already present) and 'b' (to be pushed) and return:
 * 'cmp_left' if 'b' should be inserted to the left of 'a',
 * 'cmp_right' if 'b' should be inserted to the right of 'a',
 * 'cmp_reject' if 'b' should not be inserted at all,
 * 'cmp_accept' if 'b' should be put immediately at right the current value. The same as 'cmp_right'.
 * Remember that queues pop from the left, so the more to left, the more important.
 * Pass 'NULL' if no priority check is desired.
*/
list_t *list_new_custom_queue(bool takeOwnership, cmpFunc priorityCmp);

/**
 * @brief creates a new list that behaves like a stack
 * @param takeOwnership: true if pushed values should be freed along with the list deallocation 
 * @param priorityCmp: comparison function to determine the priority of the value to be inserted.
 * Should cast and compare 'a' (already present) and 'b' (to be pushed) and return:
 * 'cmp_left' if 'b' should be inserted to the left of 'a',
 * 'cmp_right' if 'b' should be inserted to the right of 'a',
 * 'cmp_reject' if 'b' should not be inserted at all,
 * 'cmp_accept' if 'b' should be put immediately at right the current value. The same as 'cmp_right'.
 * Remember that stacks pop from the right, so the more to right, the more important.
 * Pass 'NULL' if no priority check is desired.
*/
list_t *list_new_custom_stack(bool takeOwnership, cmpFunc priorityCmp);

/**
 * @brief destroys an list. If it was created with 'takeOwnership' as true, then values inside will be deallocated alongside the list
*/
void list_destroy(list_t *l);

/**
 * @brief pushes a value to the list, accordingly to the type (queue or stack) and comparison (priority and uniqueness)
 * @attention Avg: O(1), Worst: O(n)
*/
node_t *list_push(list_t *l, void *value);

/**
 * @brief pops a value from the list, accordingly to the type (queue or stack)
 * @attention O(1)
*/
void *list_pop(list_t *l);

/**
 * @brief appends two lists, both must different and must have same memory owning of their data.
 * If one owns and the other doesn't, then the function returns immediately
 * @param l: the destination list
 * @param consumed: the list to be added and then destroyed
 * @attention O(1)
*/
void list_merge(list_t *dest, list_t *consumed);

/**
 * @brief check if an value already exists on a list based on the given 'cmpFunc'
 * @param l: the list
 * @param value: the value to be compared
 * @return true if value exists
 * @attention O(n)
*/
bool list_exists(const list_t *l, void *value);

void *list_remove_node(list_t *l, node_t *node);

/**
 * @brief get list size
*/
size_t list_size(const list_t *l);

/**
 * @brief get value in list at the 'pos'
*/
void *list_at(const list_t *l, size_t pos);

/**
 * @brief return an iterator that iterates through the list in place.
 * If list is a queue, iterates from left to right.
 * If list is a stack, iterates from right to left.
*/
list_ite list_iterate(const list_t *l);

/**
 * @brief return an iterator that iterates while popping elements from the list 
*/
list_ite list_popall(list_t *l);

// ------------------------------------------------------------ Dynamic Array ------------------------------------------------------

#define MIN_ARRAY_BLOCK_SIZE 1024

typedef struct{
	void **raw;
	size_t blockSize;
	size_t allocated;
	size_t size;
	bool onws;
}array_t;

/**
 * @brief creates a new dynamic array
 * @param takeOwnership: true if pushed values should be freed along with the array deallocation 
 * @param blockSize: the block size to be allocated at the start and reallocated by each time the array overflows
*/
array_t *array_new_custom(bool takeOwnership, size_t blockSize);

/**
 * @brief creates a new dynamic array.
 * Values in the array are NOT freed when 'array_destroy' is called.
 * Reallocation is done in increments of the macro 'MIN_ARRAY_BLOCK_SIZE', by default 1024
*/
array_t *array_new(void);

/**
 * @brief destroys an array. If it was created with 'takeOwnership' as true, then values inside will be deallocated alongside the array
*/
void array_destroy(array_t *a);

/**
 * @brief get array size
*/
size_t array_size(const array_t *a);

/**
 * @brief sets the value on the array at the position.
 * If the position is already has a value then it's overwritten.
 * If overwritten and the array has ownership, then the old value is deallocated.
 * If the position is bigger than the the current allocated size, the array is expanded by the 'blockSize'.
 * If the position is bigger than the the current array size, the array size is expanded, this leaves empty values behind!
 * @return true if set, false otherwise
 * @attention O(1)
*/
bool array_set(array_t *a, size_t pos, void *value);

/**
 * @brief adds a new value to end of the array 
 * @return the position the value was inserted
 * @attention O(1)
*/
size_t array_add(array_t *a, void *value);

/**
 * @brief return a value at the position
 * @attention O(1)
*/
void *array_get(const array_t *a, size_t pos);

/**
 * @brief removes and returns a value at the position
 * @attention O(1)
*/
void *array_remove(array_t *a, size_t pos);

/**
 * @brief removes and returns a value at the position while rearranging the array as not to leave an empty space
 * @attention O(n)
*/
void *array_pop(array_t *a, size_t pos);

// ------------------------------------------------------------ Hash table ---------------------------------------------------------

typedef uint32_t(*hashFunction)(const uint8_t *data, size_t size, size_t max);

typedef struct kv_node_t kv_node_t;
struct kv_node_t{
	void* value;
	kv_node_t *next;
	kv_node_t *prev;
	void *key;
	size_t keySize;
};

typedef struct{
	kv_node_t **values;
	uint32_t size;
	bool onws;
	hashFunction hash;
	int collisions;
}hashtable_t;

/**
 * @brief creates a new hashtable that uses djb2 as a hash function
 * Values in the hashtable are NOT freed when 'hashtable_destroy' is called
 * @param size: the size of the table, the more the less conflicts
*/
hashtable_t *hashtable_new(size_t size);

/**
 * @brief creates a new hashtable that uses a defined hash function
 * @param size: the size of the table, the more the less conflicts
 * @param takeOwnership: true if inserted values should be freed along with the hashtable deallocation 
 * @param f: a hash function. Pass NULL to use the default one
*/
hashtable_t *hashtable_new_custom(size_t size, bool takeOwnership, hashFunction f);

/**
 * @brief destroys an hashtable. If it was created with 'takeOwnership' as true, then values inside will be deallocated alongside the hashtable
*/
void hashtable_destroy(hashtable_t *ht);

/**
 * @brief set a value in the hashtable using a string key.
 * If value is present then it's not set
 * @return the hash of the key
 * @attention Avg: O(1), Worst: O(n)
*/
uint32_t hashtable_set(hashtable_t *h, char *key, void *value);

/**
 * @brief set a value in the hashtable using a binary value 'key' of size 'keySize'
 * If value is present then it's not set
 * @return the hash of the key
 * @attention Avg: O(1), Worst: O(n)
*/
uint32_t hashtable_set_bin(hashtable_t *h, void *key, size_t keySize, void *value);

/**
 * @brief get a value in the hashtable using a string key
 * @return return NULL if not value is found
 * @attention Avg: O(1), Worst: O(n)
*/
void *hashtable_get(hashtable_t *h, char *key);

/**
 * @brief get a value in the hashtable using a binary value 'key' of size 'keySize'
 * @return return NULL if not value is found
 * @attention Avg: O(1), Worst: O(n)
*/
void *hashtable_get_bin(hashtable_t *h, void *key, size_t keySize);

/**
 * @brief removes and returns a value in the hashtable using a string key
 * @return return NULL if not value is found
 * @attention Avg: O(1), Worst: O(n)
*/
void *hashtable_remove(hashtable_t *h, char *key);

/**
 * @brief removes and returns a value in the hashtable using a binary value 'key' of size 'keySize'
 * @return return NULL if not value is found
 * @attention Avg: O(1), Worst: O(n)
*/
void *hashtable_remove_bin(hashtable_t *h, void *key, size_t keySize);

/**
 * @brief check if a value exists using a string key
 * @attention Avg: O(1), Worst: O(n)
*/
bool hashtable_exists(hashtable_t *h, char *key);

/**
 * @brief check if a value exists using a binary value 'key' of size 'keySize'
 * @attention Avg: O(1), Worst: O(n)
*/
bool hashtable_exists_bin(hashtable_t *h, void *key, size_t keySize);

// ------------------------------------------------------------ Dictionary ---------------------------------------------------------

/**
 * @brief a dict is an 'list_t' that holds 'key_value_t*' inside it, and a 'hashtable_t' to map 'void*' keys back to 'list_t' 'node_t's
*/
typedef struct{
	list_t *l;
	hashtable_t *h;
}dict_t;

/**
 * @brief creates a new dictionary with pre defined hashtable size of '1024'.
 * Values in the dictionary are NOT freed when 'dict_destroy' is called
*/
dict_t *dict_new();

/**
 * @brief creates a new dictionary with specified hashtable size.
 * @param size: The underlying hashtable size
 * @param takeOwnership: true if inserted values should be freed along with the dictionary deallocation 
*/
dict_t *dict_new_custom(size_t size, bool takeOwnership);

/**
 * @brief destroys an dictionary. If it was created with 'takeOwnership' as true, then values inside will be deallocated alongside the dictionary
*/
void dict_destroy(dict_t *d);

size_t dict_size(const dict_t *d);

int64_t dict_add(dict_t *d, char *key, void *value);

int64_t dict_add_bin(dict_t *d, void *key, size_t keySize, void *value);

key_value_t dict_set(dict_t *d, size_t pos, char *key, void *value);

key_value_t dict_set_bin(dict_t *d, size_t pos, void *key, size_t keySize, void *value);


void *dict_get(const dict_t *d, char *key);

void *dict_get_bin(const dict_t *d, void *key, size_t keySize);

key_value_t dict_get_at(const dict_t *d, size_t pos);


void *dict_remove(dict_t *d, char *key);

void *dict_remove_bin(dict_t *d, void *key, size_t keySize);

key_value_t dict_remove_at(dict_t *d, size_t pos);


bool dict_exists(const dict_t *d, char *key);

bool dict_exists_bin(const dict_t *d, void *key, size_t keySize);

bool dict_exists_at(const dict_t *d, size_t pos);


typedef struct dict_ite dict_ite;
typedef key_value_t (*dict_ite_next_func)(dict_ite *ite);
struct dict_ite{
	dict_ite_next_func next;
	bool yield;
	dict_t *d;
	node_t *node;
};

/**
 * @brief return an iterator that iterates in place through the dictionary 
*/
dict_ite dict_iterate(const dict_t *d);

#endif