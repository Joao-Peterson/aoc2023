#ifndef _ITE_HEADER_
#define _ITE_HEADER_

// typedef struct ite_t ite_t;

// typedef void* (*ite_next_func) (ite_t *ite);

// struct ite_t{
// 	ite_next_func next;
// };

#define foreach(type, var, ite) for(type var = ite.next(&ite); ite.yield; var = ite.next(&ite))

#define ite_next(ite) ite.next(&ite)

// for(char *line = next(ite); line!= NULL; line = next(ite)){
	
// }

// foreach(char*, line, ite){
	
// }

#endif