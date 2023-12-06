#ifndef _STRMATCH_HEADER_
#define _STRMATCH_HEADER_

#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

// node in the string mathing tree
typedef struct charnode_t charnode_t;
struct charnode_t{
	// final word
	char *token;
	// final word as number
	int value;
	// array of pointer indexed by char
	charnode_t *nodes[UINT8_MAX];
};

// strmatch struct
typedef struct{
	charnode_t *tree;
}strmatch_t;

charnode_t *charnode_new(char *token, int value){
	charnode_t *n = (charnode_t *)calloc(sizeof(charnode_t), 1);
	n->token = token;
	n->value = value;
	return n;
}

// save token onto tree
void charnode_save_token(charnode_t *n, char *token, size_t pos, int value){
	if(token[pos] == '\0') return;

	// if char node doesnt exist
	if(n->nodes[(int)token[pos]] == NULL){
		// on last char, burn final value
		if(token[pos+1] == '\0')
			n->nodes[(int)token[pos]] = charnode_new(token, value);
		else
			n->nodes[(int)token[pos]] = charnode_new(NULL, 0);
	}

	charnode_save_token(n->nodes[(int)token[pos]], token, pos + 1, value);
}

// compile tree
void strmatch_compile(strmatch_t *sm, size_t qty, va_list tokens){
	for(size_t i = 0; i < qty; i++){
		char *token = va_arg(tokens, char*);
		int value = va_arg(tokens, int);
		charnode_save_token(sm->tree, token, 0, value);
	}
}

// create new strmatch
strmatch_t *strmatch_new(size_t tokens_qty, ...){
	va_list args;
	va_start(args, tokens_qty);

	// strmatch and root ndoe
	strmatch_t *sm = calloc(sizeof(strmatch_t), 1);
	sm->tree = charnode_new(NULL, 0);

	// compile
	strmatch_compile(sm, tokens_qty, args);
	
	va_end(args);
	return sm;
}

// recusively destroy nodes
void charnode_destroy(charnode_t *n){
	if(n != NULL){
		for(size_t i = 0; i < UINT8_MAX; i++){
			charnode_destroy(n->nodes[i]);
		}

		free(n);
	}
}

// terminate strmatch
void strmatch_destroy(strmatch_t *sm){
	charnode_destroy(sm->tree);
	free(sm);
}

// compare string to tree
charnode_t *charnode_match_token(charnode_t *n, char *token, size_t pos){
	// on end return token
	if(n->token != NULL) return n;

	// if char exists on tree, call recusively
	if(n->nodes[(int)token[pos]] != NULL)
		return charnode_match_token(n->nodes[(int)token[pos]], token, pos + 1);
	// if not, then no match
	else
		return NULL;
}

// match and return first one 
charnode_t *strmatch_match(strmatch_t *sm, char *string){
	if(string == NULL) return NULL;
	size_t l = strlen(string);

	for(size_t i = 0; i < l; i++){
		// match against every char on string
		charnode_t *match = charnode_match_token(sm->tree, &(string[i]), 0);

		// on first match return
		if(match != NULL) return match;
	}

	// no match at the end, return null 
	return NULL;
}

// match at the start and return first one 
charnode_t *strmatch_match_atstart(strmatch_t *sm, char *string){
	if(string == NULL) return NULL;

	charnode_t *match = charnode_match_token(sm->tree, string, 0);

	// on first match return
	if(match != NULL)
		return match;
	else
		return NULL;
}

#endif