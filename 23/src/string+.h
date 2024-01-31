#ifndef _STRING_HEADER_
#define _STRING_HEADER_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdint.h>
#include "data.h"
#include "error.h"
#include "ite.h"

// ------------------------------------------------------------ Defines ------------------------------------------------------------

// ammount of data to intialize strings and realloc by
#define STRING_ALLOCATION_CHUNK 1024

// ammount of characters to return from split iterator
#define STRING_SPLIT_MAX_SIZE 2048

// ansi escape define foreground color
#define FOREGROUND_COLOR "\x1b[38;2;%d;%d;%dm"

// ansi escape define background color
#define BACKGROUND_COLOR "\x1b[48;2;%d;%d;%dm"

// ansi escape reset color
#define RESET_COLOR "\x1b[m"

// ------------------------------------------------------------ Types --------------------------------------------------------------

/**
 * @brief string struct
*/
typedef struct{
	char *raw;
	size_t len;
	bool owns;
	size_t allocated;
}string;

/**
 * @brief string iterator type. Created by functions like: 'string_split', etc.
 * Can be used with subsequent calls of 'string_next(&string_ite)' to get new values from the iterator. 
 * Remember to 'string_destroy' the returns of 'string_next'
*/
typedef struct string_ite string_ite;
typedef string (*string_ite_next_func)(string_ite *ite);
struct string_ite{
	string_ite_next_func next;
	bool yield;
	const char *tokens;
	char *base;
	char *state;
	size_t max;
};

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
 * @brief sprintf like function that return a new string after formatting, receives a va_list instead of ...
*/
string *string_vsprint(const char *fmt, size_t max_size, va_list args);

/**
 * @brief sprintf like function that return a new string after formatting
*/
string *string_sprint(const char *fmt, size_t max_size, ...);

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

// ------------------------------------------------------------ Destructors --------------------------------------------------------

/**
 * @brief deallocate a string
*/
void string_destroy(string *str);

/**
 * @brief return c string and free string structure 
*/
char *string_unwrap(string *str);

// ------------------------------------------------------------ Ref / Slice --------------------------------------------------------

/**
 * @brief returns a new string from a string range
 * @param str: source string
 * @param from: source string start point, negative if counting left from the end
 * @param len: how long to slice, 0 if all to the right of 'from'
 * @return string slice of the original
*/
string string_slice(const string *str, int from, unsigned int len);

/**
 * @brief split string based on token list
 * @param string: string to split
 * @param tokens: string containing tokens
 * @return string iterator, use the 'iterator.next(&iterator)' member to grab the string slices until 'iterator.yield' becomes false. Try using the 'foreach' macro!
 * @details
 * string *myString = string_from("Hello this is a\nstring");
 * string_ite ite = string_plit(myString, " \n");
 * foreach(string, split, ite){
 * 		split_printLn(&split);
 * }
*/
string_ite string_split(const string *str, const char *tokens);

// ------------------------------------------------------------ Functions ----------------------------------------------------------

/**
 * @brief return string length inside string. This is O(1)
*/
size_t string_length(const string* str);

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
 * @brief print string to stdout
*/
void string_print(const string *str);

/**
 * @brief copy from string to string
*/
void string_copy_to(string *to, const string *from, size_t len);

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
 * @param dest: destination string
 * @param src: source cstring
 * @param len: how much characters to concatenate. if 0 or bigger than the 'src' len, copy all of 'src'
*/
void string_cat_raw(string * restrict dest, const char * restrict src, size_t len);

/**
 * @brief concatenate string src to the dest string
 * @param dest: destination string
 * @param src: source string
 * @param len: how much characters to concatenate. if 0 or bigger than the 'src' len, copy all of 'src'
*/
void string_cat(string * restrict dest, const string * restrict src, size_t len);

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

/**
 * @brief replace all 'regex' occurances in 'str' with the pattern 'replace'
 * @param str: string to process
 * @param regex: regex to use. Uses 'REG_EXTENDED' and 'REG_NEWLINE' by default
 * @param replace: replace pattern string. Use '$n' to substitute in the 'n' matched group. Maximum of a 100 groups  
 * @param maxReplaces: maximum number of occurances. Pass '0' to match as many as possible
 * @param error: Pass an '&error_t' to receive error data. Pass NULL to ignore 
 * @return a new string with all occurances replaced
*/
string *string_replaceAll(const string *str, const char *regex, const char *replace, size_t maxReplaces, error_t *error);

/**
 * @brief replace the first 'regex' occurance in 'str' with the pattern 'replace'
 * @param str: string to process
 * @param regex: regex to use. Uses 'REG_EXTENDED' and 'REG_NEWLINE' by default
 * @param replace: replace pattern string. Use '$n' to substitute in the 'n' matched group. Maximum of a 100 groups  
 * @param error: Pass an '&error_t' to receive error data. Pass NULL to ignore 
 * @return a new string with the first occurance replaced
*/
string *string_replace(const string *str, const char *regex, const char *replace, error_t *error);

/**
 * @brief return all 'regex' occurances in 'str'
 * @param str: string to process
 * @param regex: regex to use. Uses 'REG_EXTENDED' and 'REG_NEWLINE' by default
 * @param maxMatches: maximum number of occurances. Pass '0' to match as many as possible
 * @param error: Pass an '&error_t' to receive error data. Pass NULL to ignore 
 * @return returns an 'array_t' of 'string*' slices from the original string, Don't alter the strings! and remember to free the return with 'array_destroy()'
*/
array_t *string_matchAll(const string *str, const char *regex, size_t maxMatches, error_t *error);

/**
 * @brief return the first 'regex' occurance in 'str'
 * @param str: string to process
 * @param regex: regex to use. Uses 'REG_EXTENDED' and 'REG_NEWLINE' by default
 * @param error: Pass an '&error_t' to receive error data. Pass NULL to ignore 
 * @return returns a string slice from the original string, Don't alter it!
*/
string string_match(const string *str, const char *regex, error_t *error);

#endif
