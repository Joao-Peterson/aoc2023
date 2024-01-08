#ifndef _HASH_HEADER_
#define _HASH_HEADER_

#include <stdint.h>
#include <string.h>

/**
 * @brief Calculate djb2 hash of null terminated string
*/
uint64_t djb2_hash_string(const unsigned char *str);

/**
 * @brief Calculate djb2 hash
*/
uint64_t djb2_hash(const uint8_t *data, size_t size);

#endif
