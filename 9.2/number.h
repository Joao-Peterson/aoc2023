#ifndef _NUMBER_HEADER_
#define _NUMBER_HEADER_

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#define NUMBER_CONSTRUCT_SIZE 50

typedef struct{
	uint8_t base;												/**< number base */
	bool sign;													/**< true if positive, false if negative */
	uint16_t digits[NUMBER_CONSTRUCT_SIZE];  					/**< number digits. From least to most significant */
	uint16_t digitsSize;
	uint16_t mantissa[NUMBER_CONSTRUCT_SIZE];					/**< mantissa digits. From least to most significant */
	uint16_t mantissaSize;
}number_t;

/**
 * @brief get number construct base 10 from unsigned integer
*/
number_t number_from_uint(uint64_t value);

/**
 * @brief get number construct base 10 from integer
*/
number_t number_from_int(int64_t value);

/**
 * @brief get number construct base 10 from double
*/
number_t number_from_float(double value);

/**
 * @brief get number construct base 10 from string of numbers. Format: '([0-9])+.?([0-9])+\s*'
*/
number_t number_from_string_numbers(const char *value);

/**
 * @brief get number construct base 16 from string of numbers. Format: '([a-zA-Z0-9])+.?([a-zA-Z0-9])+\s*'
*/
number_t number_from_string_hex(const char *value);

/**
 * @brief get number construct base 26 from string of numbers. Format: '([a-zA-Z])+.?([a-zA-Z])+\s*'
*/
number_t number_from_string_alphabet(const char *value);

/**
 * @brief converts a number construct from one base no another
*/
number_t number_convert(number_t num, uint8_t base);

/**
 * @brief prints visual representation of the number construct using hexadecimal symbols
*/
char *number_to_hexstring(number_t num);

/**
 * @brief prints visual representation of the number construct using the alphabet from A-Z
*/
char *number_to_string_alphabet(number_t num);

/**
 * @brief get unsigned integer from number, assumes base 10. Ex: base 16 BA -> 11*(16**1) + 10*(16**0)
*/
uint64_t number_to_uint(number_t num);

/**
 * @brief get integer from number, assumes base 10. Ex: base 16 BA -> 11*(16**1) + 10*(16**0)
*/
int64_t number_to_int(number_t num);

/**
 * @brief get double from number, assumes base 10. Ex: base 16 BA -> 11*(16**1) + 10*(16**0)
*/
double number_to_float(number_t num);

/**
 * @brief find the greatestCommonDivisor using euclid's method 
*/
unsigned int greatestCommonDivisor(unsigned int a, unsigned int b);

/**
 * @brief find the leastCommonMultiple using euclid's greatestCommonDivisor
*/
unsigned int leastCommonMultiple(unsigned int a, unsigned int b);

/**
 * @brief find the leastCommonMultiple of a list of numbers using the table method with primes 
*/
uint64_t leastCommonMultipleN(uint64_t *values, size_t size);

#endif