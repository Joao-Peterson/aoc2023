#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include "src/worker.h"
#include "src/string+.h"
#include "src/number.h"
#include "src/hash.h"
#include "src/data.h"

void printMatrix(char **m, size_t w, size_t h){
	for(size_t i = 0; i < h; i++){
		printf("%.*s\n", (int)w, m[i]);
	}
}

typedef struct module_t module_t;

struct module_t{
	string* name;
	size_t id;
	char type;
	int state[100];
	module_t *modules[100];
	size_t modulesSize;
	array_t *moduleNames;
};

typedef struct{
	module_t *from;
	bool signal;
	module_t *module;
}signal_t;

typedef struct{
	module_t *broadcaster;
	module_t *rx_in;
	array_t *rx_inputs;
}configuration_t;

array_t *parseModulesNames(const string *str){
	array_t *a = array_new();

	string_ite_t ite = string_split(str, ",");
	for(string *name = string_next(&ite); name != NULL; name = string_next(&ite)){
		array_add(a, name);
	}

	return a;
}

// parse file
configuration_t *parseInput(const string *data){

	configuration_t *conf = calloc(1, sizeof(configuration_t));
	conf->rx_inputs = array_new();
	array_t *names = array_new();
	hashtable_t *modules = hashtable_new(100000, false, NULL);

	string *dd = string_replaceAll(data, " -> ", ";", 0, NULL);
	string *dataNew = string_replaceAll(dd, ", ", ",", 0, NULL);
	string_ite_t iterator = string_split(dataNew, "\n");
	for(string *line = string_next(&iterator); line != NULL; line = string_next(&iterator)){
		module_t *mod = calloc(1, sizeof(module_t));
		
		string_ite_t comma = string_split(line, ";");
		
		// name && symbol
		string *name = string_next(&comma);
		if(!isalnum(name->raw[0])){
			mod->type = name->raw[0];
			name = string_slice(name, 1, 0);
		}
		else
			mod->type = 0;

		mod->name = name;

		// modules of mod
		string *modNames = string_next(&comma);
		mod->moduleNames = parseModulesNames(modNames);

		// all high for conjunction
		if(mod->type == '&'){
			for(size_t m = 0; m < 100; m++)
				mod->state[m] = 1;
		}

		array_add(names, name);
		hashtable_set(modules, name->raw, mod);

		string_destroy(modNames);
		string_destroy(line);
	}

	// create tree
	for(size_t i = 0; i < names->size; i++){
		string *name = array_get(names, i);
		module_t *mod = hashtable_get(modules, name->raw)->value;
		mod->id = i;

		if(string_cmp_raw(mod->name, "broadcaster"))
			conf->broadcaster = mod;

		mod->modulesSize = mod->moduleNames->size;
		for(size_t m = 0; m < mod->moduleNames->size; m++){
			string *n = array_get(mod->moduleNames, m);
			node_t *node = hashtable_get(modules, n->raw);

			module_t *submod;
			// ending node
			if(node == NULL){
				module_t *final = calloc(1, sizeof(module_t));
				final->type = '*';
				final->name = n;
				submod = final;

				if(string_cmp_raw(n, "rx"))
					conf->rx_in = mod;
			}
			else{
				submod = node->value;
			}
			
			mod->modules[m] = submod;

			// zero this module slot if input of conjunction 
			if(mod->modules[m]->type == '&')
				mod->modules[m]->state[mod->id] = 0;
		}
	}

	// get rx inputs
	// create tree
	for(size_t i = 0; i < names->size; i++){
		string *name = array_get(names, i);
		module_t *mod = hashtable_get(modules, name->raw)->value;

		for(size_t m = 0; m < mod->modulesSize; m++){
			if(mod->modules[m] == conf->rx_in)
				array_add(conf->rx_inputs, mod);
		}
	}

	// array_destroy(names);
	// hashtable_destroy(modules);
	string_destroy(dd);
	string_destroy(dataNew);
	return conf;
}

void signal_push(queue_t *q, bool signal, module_t *from, module_t *module){
	signal_t *s = calloc(1, sizeof(signal_t));
	s->from = from;
	s->signal = signal;
	s->module = module;

	queue_push(q, s);
}

signal_t *signal_pop(queue_t *q){
	return queue_pop(q);
}

uint64_t part1(const configuration_t *c){
	uint64_t lows = 0, highs = 0;
	queue_t *sq = queue_new(true);

	for(size_t b = 0; b < 1000; b++){
		// button
		signal_push(sq, 0, NULL, c->broadcaster);

		for(signal_t *sig = signal_pop(sq); sig != NULL; sig = signal_pop(sq)){
			if(sig->signal)
				highs++;
			else
				lows++;

			// printf("%s -%s-> %s\n", sig->from == NULL ? "NULL" : sig->from->name->raw, sig->signal ? "high" : "low", sig->module->name->raw);

			// send signals
			switch(sig->module->type){
				// flip flop
				case '%':
					if(sig->signal == 1)
						break;
				
					sig->module->state[0] ^= 1; // flip

					for(size_t i = 0; i < sig->module->modulesSize; i++)
						signal_push(sq, sig->module->state[0], sig->module, sig->module->modules[i]);
				break;

				// conjunction
				case '&':
					sig->module->state[sig->from->id] = sig->signal;

					bool all = true;
					for(size_t j = 0; j < 100; j++){
						if(sig->module->state[j] != 1){
							all = false;
							break;
						}
					}
				
					for(size_t i = 0; i < sig->module->modulesSize; i++)
						signal_push(sq, !all, sig->module, sig->module->modules[i]);
				break;

				// pass through
				case 0:
					for(size_t i = 0; i < sig->module->modulesSize; i++)
						signal_push(sq, sig->signal, sig->module, sig->module->modules[i]);
				break;

				default:
				break;
			}

			free(sig);
		}
	}

	return highs * lows;
}

int64_t mod_exists(const array_t *modules, const module_t *mod){
	for(size_t i = 0; i < modules->size; i++){
		if(array_get(modules, i) == mod)
			return i;
	}

	return -1;
}

uint64_t part2(const configuration_t *c){
	queue_t *sq = queue_new(true);
	size_t cnt = 0;
	uint64_t diff[100] = {0};
	uint64_t last[100] = {0};
	uint64_t presses[100] = {0};

	for(size_t b = 0; cnt < c->rx_inputs->size; b++){
		// button
		signal_push(sq, 0, NULL, c->broadcaster);

		for(signal_t *sig = signal_pop(sq); sig != NULL; sig = signal_pop(sq)){
			if(sig->module == c->rx_in && sig->signal == 1){
				int64_t p = mod_exists(c->rx_inputs, sig->from);

				if(presses[p] == 0){
					// wait to stabilize
					if(b + 1 - last[p] == diff[p]){
						presses[p] = diff[p];
						cnt++;
					}
					else{
						diff[p] = b + 1 - last[p];
						last[p] = b + 1;
					}
				}
			}
			
			// send signals
			switch(sig->module->type){
				// flip flop
				case '%':
					if(sig->signal == 1)
						break;
				
					sig->module->state[0] ^= 1; // flip

					for(size_t i = 0; i < sig->module->modulesSize; i++)
						signal_push(sq, sig->module->state[0], sig->module, sig->module->modules[i]);
				break;

				// conjunction
				case '&':
					sig->module->state[sig->from->id] = sig->signal;

					bool all = true;
					for(size_t j = 0; j < 100; j++){
						if(sig->module->state[j] != 1){
							all = false;
							break;
						}
					}
				
					for(size_t i = 0; i < sig->module->modulesSize; i++)
						signal_push(sq, !all, sig->module, sig->module->modules[i]);
				break;

				// pass through
				case 0:
					for(size_t i = 0; i < sig->module->modulesSize; i++)
						signal_push(sq, sig->signal, sig->module, sig->module->modules[i]);
				break;

				default:
				break;
			}

			free(sig);
		}
	}

	return leastCommonMultipleN(presses, c->rx_inputs->size);
}

int main(int argc, char**argv){
	string *data;
	
	// from arg filename
	if(argc > 1){
		data = string_from_filename(argv[1], NULL);
	}
	// from pipe
	else{
		data = string_from_file(stdin, NULL);
	}

	configuration_t *configuration = parseInput(data);
	
	// too high 1334258160
	printf("Part 1: %lu\n", part1(configuration));
	// to low 22803499706691
	printf("Part 2: %lu\n", part2(configuration));

	string_destroy(data);
	return 0;
}