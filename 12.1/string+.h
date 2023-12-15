#ifndef _STRING_HEADER_
#define _STRING_HEADER_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdint.h>

// ------------------------------------------------------------ Defines ------------------------------------------------------------

// ammount of data to intialize strings and realloc by
#define STRING_ALLOCATION_CHUNK 1024

// ammount of characters to return from split iterator
#define STRING_SPLIT_MAX_SIZE 2048

// ------------------------------------------------------------ Types --------------------------------------------------------------

/**
 * @brief string struct
*/
typedef struct{
	char *raw;
	bool managed;
	size_t len;
	size_t allocated;
}string;

/**
 * @brief 
*/
typedef struct{
	const char *tokens;
	char *state;
}string_ite_t;

// ------------------------------------------------------------ Constructors -------------------------------------------------------

/**
 * @brief create new empty string
*/
string *string_new();

/**
 * @brief create new string with predefined allocated size
*/
string *string_new_sized(size_t size);

/**
 * @brief create a copy from another string
*/
string *string_copy(const string *str);

/**
 * @brief create a copy from another c string
*/
string *string_from(const char *str);

/**
 * @brief create a new string that contains the c string within, wrapping it
 * @param string: c string
 * @param take_ownership: if true, then the original c string will be deallocated on string_destroy call 
*/
string *string_wrap(char *string, bool take_ownership);

/**
 * @brief create string from FILE pointer
 * @param file: FILE* descriptor. Made by using fopen
 * @param read: size read form file
*/
string *string_from_file(FILE *file, size_t *read);

/**
 * @brief create string from a file named filename
 * @param filename: the filesystem name of the file relative to executable
 * @param read: size read form file
*/
string *string_from_filename(const char *filename, size_t *read);

/**
 * @brief returns a new string from a string range
 * @param str: source string
 * @param from: source string start
 * @param len: how long
 * @return newly allocated string
 * @warning the result should be freed manually 
*/
string *string_slice(const string *str, size_t from, size_t len);

// ------------------------------------------------------------ Destructors --------------------------------------------------------

/**
 * @brief deallocate a string
*/
void string_destroy(string *str);

/**
 * @brief return c string and free string structure 
*/
char *string_unwrap(string *str);

// ------------------------------------------------------------ Functions ----------------------------------------------------------

/**
 * @brief return string length inside string. This is O(1)
*/
size_t string_length(const string* str);

/**
 * @brief sprintf like function that return a new string after formatting, receives a va_list instead of ...
*/
string *string_vsprint(const char *fmt, size_t max_size, va_list args);

/**
 * @brief sprintf like function that return a new string after formatting
*/
string *string_sprint(const char *fmt, size_t max_size, ...);

/**
 * @brief write formatted text to string
*/
void string_write(string *str, const char *fmt, size_t buffer_size, ...);

/**
 * @brief write formatted text to string
*/
void string_writeLn(string *str, const char *fmt, size_t buffer_size, ...);

/**
 * @brief write formatted text to string, receives va_list instead off ...
*/
void string_vwrite(string *str, const char *fmt, size_t buffer_size, va_list args);

/**
 * @brief write formatted text to string, receives va_list instead off ...
*/
void string_vwriteLn(string *str, const char *fmt, size_t buffer_size, va_list args);

/**
 * @brief print string to stdout as new line
*/
void string_println(const string *str);

/**
 * @brief compare string to another one
*/
bool string_cmp(const string *a, const string *b);

/**
 * @brief compare string to a c string
*/
bool string_cmp_raw(const string *a, const char *b);

/**
 * @brief concatenate c string src to the dest string
*/
void string_cat_raw(string *dest, const char *src);

/**
 * @brief concatenate string src to the dest string
*/
void string_cat(string *dest, const string *src);

/**
 * @brief split string based on token list
 * @param string: string to split
 * @param tokens: string containing tokens
 * @return string iterator, use string_ite_next to grab the string parts until it return NULL
 * @details
 * string *myString = string_from("Hello this is a\nstring");
 * string_ite_t ite = string_plit(myString, " \n");
 * for(string *split = string_next(&ite); split != NULL; split = string_next(&ite)){
 * 		split_printLn(split);
 * 		string_destroy(split);
 * }
*/
string_ite_t string_split(const string *str, const char *tokens);

/**
 * @brief return parts of the string based on the iterator
 * @warning call string_destroy on each value returned by the call
 * @param iterator: pointer to iterator
 * @return dynamically allocated string type
 * @details
 * string *myString = string_from("Hello this is a\nstring");
 * string_ite_t ite = string_plit(myString, " \n");
 * for(string *split = string_next(&ite); split != NULL; split = string_next(&ite)){
 * 		split_printLn(split);
 * 		string_destroy(split);
 * }
*/
string *string_next(string_ite_t *iterator);

/**
 * @brief convert string to int
 * @param str: a string
 * @param base: the characters to be used in the representation. Ex: 2 for "01", 10 for "0123456789", 16 for "0123456789ABCDEF" ...
*/
int64_t string_to_int(const string *str, uint32_t base);

/**
 * @brief convert string to uint
 * @param str: a string
 * @param base: the characters to be used in the representation. Ex: 2 for "01", 10 for "0123456789", 16 for "0123456789ABCDEF" ...
*/
uint64_t string_to_uint(const string *str, uint32_t base);

/**
 * @brief convert string to double
*/
double string_to_double(const string *str);

#endif
