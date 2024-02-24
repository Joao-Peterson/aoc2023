#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <regex.h>
#include <math.h>
#include "src/worker.h"
#include "src/string+.h"
#include "src/number.h"
#include "src/hash.h"
#include "src/data.h"
#include "src/matrix.h"
#include "src/ite.h"

typedef struct{
	matrix_t *graph;
}puzzle_t;

typedef struct{
	int minCut;
	int *vertices;
	size_t verticesSize;
}cut_t;

typedef struct{
	int id;
	string name;
	string consNames;
}con_t;

puzzle_t *parseInput(const string *data){
	puzzle_t *p = calloc(1, sizeof(puzzle_t));
	dict_t *graph = dict_new_custom(2000, false);

	// unique nodes count:
	// cat test.txt | sed -E 's/:?\s/\n/g' | sort | uniq | wc -l

	int i = 0;
	string_ite ite = string_split(data, "\n");
	foreach(string, line, ite){
		string_ite lineite = string_split(&line, ":");

		con_t *con = malloc(sizeof(con_t));
		string name = ite_next(lineite);

		*con = (con_t){
			.id = i++,
			.name = name,
			.consNames = ite_next(lineite)
		};

		dict_add_bin(graph, name.raw, name.len, con);
	}

	// add missing nodes
	dict_ite di = dict_iterate(graph);
	foreach(key_value_t, kv, di){
		con_t *con = kv.value;
		if(con->consNames.raw == NULL) continue;

		// for each node connections
		string_ite conite = string_split(&con->consNames, " ");
		foreach(string, contoken, conite){
			con_t *ref = dict_get_bin(graph, contoken.raw, contoken.len);

			// if no previous entry, add to graph
			if(ref == NULL){
				ref = calloc(1, sizeof(con_t));
				ref->id = i++;
				ref->name = contoken;
				dict_add_bin(graph, contoken.raw, contoken.len, ref);
			}
		}
	}

	// make matrix
	p->graph = matrix_new(graph->l->size, graph->l->size, 0);
	di = dict_iterate(graph);
	foreach(key_value_t, kv, di){
		con_t *con = kv.value;
		if(con->consNames.raw == NULL) continue;

		// for each node add connections
		string_ite conite = string_split(&con->consNames, " ");
		foreach(string, contoken, conite){
			con_t *to = dict_get_bin(graph, contoken.raw, contoken.len);
			p->graph->rows[con->id][to->id] = 1;
			p->graph->rows[to->id][con->id] = 1;
		}
	}

	// string *m = matrix_print_int(p->graph, 1, ' ', '\n');
	// string_println(m);
	// string_destroy(m);

	dict_destroy(graph);
	return p;
}

size_t max_index(int *arr, size_t size){
	int max = 0;
	size_t index = 0;
	for(size_t i = 0; i < size; i++){
		if(arr[i] > max){
			max = arr[i];
			index = i;
		}
	}

	return index;
}

// based on: https://en.wikipedia.org/wiki/Stoer%E2%80%93Wagner_algorithm#Example_code
cut_t stoer_wagner_mincut(matrix_t *graph){
	if(graph->h != graph->w) return(cut_t){0};

    int nodeIndex = -1;
    int minCut = INT32_MAX;
    size_t n = graph->w;
    matrix_t *setsOfNodes = matrix_new(n, n, 0);
	size_t *coSize = calloc(sizeof(size_t), n);

    for(size_t i = 0; i < n; i++){
        setsOfNodes->rows[i][0] = i;
        coSize[i] = 1;
	}

	for(size_t cut = 1; cut < n; cut++){
		// use first node as the supernode. Don't alter original graph
        int *w = malloc(sizeof(int) * n);
		memcpy(w, graph->rows[0], n * sizeof(int));

		// merge nodes
        size_t s = 0, t = 0;
        for(size_t merge = 0; merge < n - cut; merge++){
			// remove the current node from the merge. Minus infinity cause when we merge later, the addition nevers turns this into a positive number again
            w[t] = INT32_MIN;

			// save last cut
            s = t;

			// find max weighted edge:
			// Without p. queue: O(V^2)
			// With    p. queue: O(E log V)
			t = max_index(w, n);
			
			// merge edges of node t into super node
			for(size_t i = 0; i < n; i++)
                w[i] += graph->rows[t][i];
        }

		// check if current cut is minimum
        int weight = w[t] - graph->rows[t][t];
        if(weight < minCut){
            minCut = weight;
			// remember node
            nodeIndex = t;
        }

		// remember connected nodes
		for(size_t i = 0; i < coSize[t]; i++)
			setsOfNodes->rows[s][coSize[s] + i] = setsOfNodes->rows[t][i];
		coSize[s] += coSize[t];
		
		// merge last and second last nodes
		for(size_t i = 0; i < n; i++){
			// on row
            graph->rows[s][i] += graph->rows[t][i];
			// and col
            graph->rows[i][s] = graph->rows[s][i];
        }

		// printf("Cut %4d | Weight: %4d | MinCut: %4d\n", cut, weight, minCut);

		// remove last edge from the graph
        graph->rows[0][t] = INT32_MIN;

		free(w);
    }

	// return mincut weight and set of nodes on one side of that cut
    return (cut_t){.minCut = minCut, .vertices = setsOfNodes->rows[nodeIndex], .verticesSize = coSize[nodeIndex]};
}

uint64_t part1(const puzzle_t *p){
	cut_t minCut = stoer_wagner_mincut(p->graph);
	return minCut.verticesSize * (p->graph->w - minCut.verticesSize);
}

uint64_t part2(const puzzle_t *p){
	return 0;
}

int main(int argc, char**argv){
	string *data;
	if(argc > 1) data = string_from_filename(argv[1], NULL);	// from arg filename
	else         data = string_from_file(stdin, NULL);      	// from pipe
	if(data == NULL) return -1;

	puzzle_t *p = parseInput(data);
	
	// string *g = graph_to_dot(p->cons);
	// string_save_to_file(g, "graph.dot");
	// string_destroy(g);

	// 2958 too low
	// 532084 too low
	printf("Part 1: %lu\n", part1(p));
	printf("Part 2: %lu\n", part2(p));

	string_destroy(data);
	return 0;
}

/**
 * ! Old approach using dict_t as a graph below:
 * Kinda of worked, but end result for input was a little bit low.
 * didn't understand why cause it worked pretty well, but the result for the input was hard to debug,
 * so a gave up and did an adjacent matrix implementation
*/

// string *dict_print(const dict_t *d){
// 	string *s = string_new();
// 	dict_ite i = dict_iterate(d);
// 	string_cat_raw(s, "{", 1);
// 	bool a = false;
// 	foreach(key_value_t, kv, i){
// 		if(a) string_cat_raw(s, ", ", 2);
// 		string_write(s, "%.*s: %p", 100, (int)kv.keySize, (char*)kv.key, kv.value);
// 		a = true;
// 	}

// 	string_cat_raw(s, "}", 1);
// 	return s;
// }

// typedef struct con_t con_t;

// typedef struct{
// 	int weight;
// 	con_t *con;
// }edge_t;

// struct con_t{
// 	string name;
// 	string consNames;

// 	// dict_t<edge_t*>
// 	dict_t* edges;
// };

// typedef struct{
// 	// dict_t<con_t*>
// 	dict_t *graph;
// }puzzle_t;

// // parse file
// puzzle_t *parseInput(const string *data){
// 	puzzle_t *p = calloc(1, sizeof(puzzle_t));
// 	p->graph = dict_new_custom(2000, false);

// 	string_ite ite = string_split(data, "\n");
// 	foreach(string, line, ite){
// 		string_ite lineite = string_split(&line, ":");

// 		con_t *con = malloc(sizeof(con_t));
// 		string name = ite_next(lineite);

// 		*con = (con_t){
// 			.name = name,
// 			.consNames = ite_next(lineite),
// 			.edges = dict_new_custom(20, true)
// 		};

// 		dict_add_bin(p->graph, name.raw, name.len, con);
// 	}

// 	// make tree
// 	dict_ite di = dict_iterate(p->graph);
// 	foreach(key_value_t, kv, di){
// 		con_t *con = kv.value;
// 		if(con->consNames.raw == NULL) continue;

// 		// for each node add connections
// 		string_ite conite = string_split(&con->consNames, " ");
// 		foreach(string, contoken, conite){
// 			con_t *ref = dict_get_bin(p->graph, contoken.raw, contoken.len);

// 			// if no previous entry, add to graph
// 			if(ref == NULL){
// 				ref = calloc(1, sizeof(con_t));
// 				ref->name = contoken;
// 				ref->edges = dict_new_custom(100, true);
// 				dict_add_bin(p->graph, contoken.raw, contoken.len, ref);
// 			}
			
// 			// add edge to con
// 			edge_t *e = malloc(sizeof(edge_t));
// 			e->con = ref;
// 			e->weight = 1;
// 			dict_add_bin(con->edges, contoken.raw, contoken.len, e);

// 			// add current to con (makes the graph undirected, cause the con can access its parent)
// 			edge_t *this = malloc(sizeof(edge_t));
// 			this->con = con;
// 			this->weight = 1;
// 			dict_add_bin(ref->edges, con->name.raw, con->name.len, this);
// 		}
// 	}

// 	return p;
// }

// typedef struct{
// 	// dict_t<con_t*>
// 	dict_t *graph;
// 	int weight;
// 	con_t *last;
// }cut_t;

// cmp_t cut_cmp(void *a, void *b){
// 	const cut_t *cuta = (cut_t*)a, *cutb = (cut_t*)b;

// 	if(cutb->weight < cuta->weight)
// 		return cmp_left;
// 	else
// 		return cmp_right;
// }

// dict_t *graph_copy(const dict_t *graph){
// 	dict_t *new = dict_new_custom(graph->h->size, true);

// 	// copy cons
// 	dict_ite ite = dict_iterate(graph);
// 	foreach(key_value_t, kv, ite){
// 		const con_t *con = kv.value;
// 		con_t *newCon = malloc(sizeof(con_t));
// 		newCon->name = con->name;
// 		dict_add_bin(new, newCon->name.raw, newCon->name.len, newCon);
// 	}

// 	// add refs
// 	ite = dict_iterate(graph);
// 	foreach(key_value_t, kv, ite){
// 		const con_t *con = kv.value;
// 		const dict_t *conEdges = con->edges;

// 		// new edges
// 		dict_t *newConEdges = dict_new_custom(conEdges->h->size, true);

// 		// copy edges
// 		dict_ite edgesIte = dict_iterate(conEdges);
// 		foreach(key_value_t, edgeKv, edgesIte){
// 			const edge_t *edge = edgeKv.value;
// 			edge_t *newEdge = malloc(sizeof(edge_t));
// 			newEdge->weight = edge->weight;
// 			newEdge->con = dict_get_bin(new, edgeKv.key, edgeKv.keySize);

// 			// add edge
// 			dict_add_bin(newConEdges, edgeKv.key, edgeKv.keySize, newEdge);
// 		}

// 		// add edge to con
// 		con_t *newCon = dict_get_bin(new, kv.key, kv.keySize);
// 		newCon->edges = newConEdges;
// 	}

// 	return new;
// }

// void graph_print(const dict_t *graph){
// 	dict_ite gite = dict_iterate(graph);
// 	foreach(key_value_t, kv, gite){
// 		printf("%.*s: ", (int)kv.keySize, (char*)kv.key);

// 		const con_t *con = kv.value;
// 		if(con->edges == NULL){
// 			printf("\n");
// 			continue;
// 		}
		
// 		dict_ite conite = dict_iterate(con->edges);
// 		foreach(key_value_t, ckv, conite){
// 			const edge_t *e = ckv.value;
// 			printf("(%d -> %.*s), ", e->weight, (int)ckv.keySize, (char*)ckv.key);
// 		}

// 		printf("\n");
// 	}
// 	printf("\n");
// }

// string *graph_to_dot(const dict_t *graph){
// 	string *ret = string_new();
// 	string_writeLn(ret, "digraph {", 10);
// 	string_writeLn(ret, "\trankdir=LR;", 25);

// 	dict_ite gite = dict_iterate(graph);
// 	foreach(key_value_t, kv, gite){
// 		const con_t *con = kv.value;
// 		if(con->edges == NULL)
// 			continue;

// 		dict_ite conite = dict_iterate(con->edges);
// 		foreach(key_value_t, ckv, conite){
// 			const edge_t *e = ckv.value;
// 			string_writeLn(ret, "\t%.*s -> %.*s[label=\"%d\",weight=\"%d\"]", 60, (int)kv.keySize, (char*)kv.key, (int)ckv.keySize, (char*)ckv.key, e->weight, e->weight);
// 		}
// 	}

// 	string_writeLn(ret, "}", 10);
// 	return ret;
// }

// // kinda of :v
// void graph_merge_nodes(dict_t *graph, con_t *con, con_t *root){
// 	// remove con from it's connected nodes 
// 	dict_ite maxConsIte = dict_iterate(con->edges);
// 	foreach(key_value_t, kv, maxConsIte){
// 		edge_t *edge = kv.value;
// 		dict_remove_bin(edge->con->edges, con->name.raw, con->name.len);
// 	}

// 	/**
// 	 * since we are not removing references of con from the all the graph edges,
// 	 * we need to remove them as we go, that means ignoring merges from already merged con's,
// 	 * but we need to remove ref's of con from the root node, otherwise it might interfere in the stoer wagner
// 	 * super node, because the invalid ref might alter the max weight found on it's edges. Trust me, I tested it! 
// 	*/

// 	// remove con from graph 
// 	dict_remove_bin(graph, con->name.raw, con->name.len);

// 	// remove con from root node
// 	dict_remove_bin(root->edges, con->name.raw, con->name.len);

// 	// add con edges to root edges
// 	if(con->edges != NULL){
// 		dict_ite maxEdgesIte = dict_iterate(con->edges);
// 		foreach(key_value_t, kv, maxEdgesIte){
// 			// if itself, don't add
// 			if(kv.keySize == root->name.len && !memcmp(kv.key, root->name.raw, kv.keySize))
// 				continue;
			
// 			// if same is edge already in, add weights
// 			if(dict_add_bin(root->edges, kv.key, kv.keySize, kv.value) == -1){
// 				edge_t *rootEdge = dict_get_bin(root->edges, kv.key, kv.keySize);
// 				const edge_t *edge = kv.value;
// 				rootEdge->weight += edge->weight;
// 			}
// 		}
// 	}
// }

// cut_t *stoer_wagner_mincut_phase(const dict_t *graph){
// 	// copy from cut graph
// 	dict_t *super = graph_copy(graph);

// 	key_value_t rootkv = dict_get_at(super, 0);
// 	con_t *root = rootkv.value;
// 	con_t *lastRemoved = NULL;

// 	// reduce. Merge root and max nodes together
// 	while(dict_size(super) > 2){
// 		// find max
// 		dict_ite rootIte = dict_iterate(root->edges);
// 		edge_t *maxedge = NULL;
// 		foreach(key_value_t, kv, rootIte){
// 			edge_t* rootedge = kv.value;
// 			if(maxedge == NULL || rootedge->weight >= maxedge->weight)
// 				maxedge = rootedge;
// 		}
// 		con_t *max = maxedge->con;
// 		lastRemoved = max;

// 		// merge max into root node 
// 		graph_merge_nodes(super, max, root);

// 		// graph_print(super);
// 	}

// 	// update current graph
// 	dict_t *new = graph_copy(graph);

// 	key_value_t lastEdgeKv = dict_get_at(root->edges, 0);
// 	edge_t *lastEdge = lastEdgeKv.value;
// 	con_t *lastConOnNew = dict_get_bin(new, lastEdgeKv.key, lastEdgeKv.keySize);
// 	con_t *secLastConOnNew = dict_get_bin(new, lastRemoved->name.raw, lastRemoved->name.len);
// 	graph_merge_nodes(new, lastConOnNew, secLastConOnNew);
// 	// graph_print(new);

// 	// cut info
// 	cut_t *cut = malloc(sizeof(cut_t));
// 	cut->weight = lastEdge->weight;
// 	cut->graph = new;
// 	cut->last = lastEdge->con;

// 	return cut;
// }

// list_t *stoer_wagner_mincut(const dict_t *graph){
// 	list_t *cuts = list_new_custom_queue(true, cut_cmp);

// 	// current graph
// 	dict_t *currentGraph = (dict_t*)graph;
// 	graph_print(currentGraph);

// 	int i = 0;

// 	// find min cut of the current cut graph
// 	while(dict_size(currentGraph) > 2){
// 		// store cut and update current
// 		cut_t *cut = stoer_wagner_mincut_phase(currentGraph);
// 		list_push(cuts, cut);
// 		currentGraph = cut->graph;

// 		printf("Graph cut [%d] | Edge: (%.*s) | Weight: %d | Size: %d\n", i++, cut->last->name.len, cut->last->name.raw, cut->weight, (int)cut->graph->l->size);
// 		// graph_print(cut->graph);
// 	}

// 	return cuts;
// }

// uint64_t part1(const puzzle_t *p){
// 	list_t *cuts = stoer_wagner_mincut(p->graph);
// 	const cut_t *min = list_pop(cuts);
// 	return (dict_size(p->graph) - dict_size(min->graph)) * dict_size(min->graph);
// }

// uint64_t part2(const puzzle_t *p){
// 	return 0;
// }

// int main(int argc, char**argv){
// 	string *data;
// 	if(argc > 1) data = string_from_filename(argv[1], NULL);	// from arg filename
// 	else         data = string_from_file(stdin, NULL);      	// from pipe
// 	if(data == NULL) return -1;

// 	puzzle_t *p = parseInput(data);
	
// 	// string *g = graph_to_dot(p->cons);
// 	// string_save_to_file(g, "graph.dot");
// 	// string_destroy(g);

// 	// 2958 too low
// 	// 532084 too low
// 	printf("Part 1: %lu\n", part1(p));
// 	printf("Part 2: %lu\n", part2(p));

// 	string_destroy(data);
// 	return 0;
// }