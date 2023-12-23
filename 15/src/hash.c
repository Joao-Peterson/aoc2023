#include "hash.h"

uint64_t djb2_hash_string(const unsigned char *str) {
    uint64_t hash = 5381;
    int c;

    while(c = *str++)
        hash = ((hash << 5) + hash) + c;
		
    return hash;
}

uint64_t djb2_hash(const uint8_t *data, size_t size){
    uint64_t hash = 5381;
	size_t i = 0;

    while(i < size){
		int c = *data++;
        hash = ((hash << 5) + hash) + c;
	}
		
    return hash;
}
