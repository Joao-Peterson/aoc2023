#include "string+.h"

// ------------------------------------------------------------ String -------------------------------------------------------------

string *string_new_sized(size_t size){
	string *str = calloc(sizeof(char), sizeof(string));
	str->len = 0;
	str->allocated = size;
	str->managed = true;

	if(str->allocated > 0 )
		str->raw = calloc(str->allocated, sizeof(char));
	else
		str->raw = NULL;

	return str;
}

string *string_new(){
	return string_new_sized(STRING_ALLOCATION_CHUNK);
}

string *string_copy(const string *str){
	if(str == NULL)
		return string_new_sized(0);

	string *new = string_new_sized(str->allocated);
	new->len = str->len;
	memcpy(new->raw, str->raw, str->len);
	return new;
}

string *string_from(const char *raw){
	if(raw == NULL)
		return string_new_sized(0);
		
	size_t len = strlen(raw);
	string *str = string_new_sized(len + 1);
	str->len = len;
	memcpy(str->raw, raw, len);
	return str;
}

string *string_wrap(char *raw, bool take_ownership){
	if(raw == NULL)
		return string_new_sized(0);
		
	size_t len = strlen(raw);
	string *str = string_new_sized(0);
	str->managed = take_ownership;
	str->len = len;
	str->raw = raw;
	return str;
}

string *string_vsprint(const char *fmt, size_t max_size, va_list args){
	char *buffer = malloc(max_size);
	vsnprintf(buffer, max_size, fmt, args);
	string *str = string_from(buffer);
	free(buffer);
	return str;
}

string *string_sprint(const char *fmt, size_t max_size, ...){
	va_list args;
	va_start(args, max_size);
	string *str = string_vsprint(fmt, max_size, args);
	va_end(args);
	return str;
}

void string_destroy(string *str){
	if(str == NULL) return;
	
	if(str->managed && str->raw != NULL)
		free(str->raw);

	free(str);
}

char *string_unwrap(string *str){
	if(str == NULL) return NULL;
	
	char *raw = str->raw;
	free(str);
	return raw;
}

void string_println(const string *str){
	printf("%s\n", str->raw == NULL ? "" : str->raw);
}

bool string_cmp(const string *a, const string *b){
	if(a->raw == NULL || b->raw == NULL) return false;
	return !strcmp(a->raw, b->raw);
}

bool string_cmp_raw(const string *a, const char *b){
	if(a->raw == NULL || b == NULL) return false;
	return !strcmp(a->raw, b);
}

void _string_cat_raw(string *dest, const char *src, size_t srclen){
	if(dest == NULL || src == NULL) return;

	size_t len = dest->len + srclen;
	if(dest->allocated == 0){
		dest->allocated = len + STRING_ALLOCATION_CHUNK;
		dest->raw = calloc(dest->allocated, sizeof(char));
		dest->len = len;
		memcpy(dest->raw, src, len);	
	}
	else{
		if(dest->allocated < (len + 1)){
			dest->allocated = len + STRING_ALLOCATION_CHUNK;
			dest->raw = realloc(dest->raw, dest->allocated);
		}

		dest->len = len;
		strncat(dest->raw, src, len);
	}
}

void string_cat_raw(string *dest, const char *src){
	if(dest == NULL || src == NULL) return;

	_string_cat_raw(dest, src, strlen(src));
}

void string_cat(string *dest, const string *src){
	if(dest == NULL || src == NULL) return;

	_string_cat_raw(dest, src->raw, src->len);
}

void string_vwrite(string *str, const char *fmt, size_t buffer_size, va_list args){
	if(str == NULL || buffer_size == 0) return;

	string *expanded = string_vsprint(fmt, buffer_size, args);
	string_cat(str, expanded);
	string_destroy(expanded);
}

void string_vwriteLn(string *str, const char *fmt, size_t buffer_size, va_list args){
	if(str == NULL || buffer_size == 0) return;
	string_vwrite(str, fmt, buffer_size, args);
	string_cat_raw(str, "\n");
}

void string_write(string *str, const char *fmt, size_t buffer_size, ...){
	va_list args;
	va_start(args, buffer_size);
	string_vwrite(str, fmt, buffer_size, args);
	va_end(args);
}

void string_writeLn(string *str, const char *fmt, size_t buffer_size, ...){
	va_list args;
	va_start(args, buffer_size);
	string_vwrite(str, fmt, buffer_size, args);
	string_cat_raw(str, "\n");
	va_end(args);
}

string *string_from_file(FILE *file, size_t *read){
    if(file == NULL) return NULL;

    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    fseek(file, 0, SEEK_SET);

    uint8_t *buffer = (uint8_t*)malloc(size + 1);
    fread(buffer, size, 1, file);

    fclose(file);

    buffer[size] = 0;

    if(read != NULL)
        *read = size;

    return string_wrap((char*)buffer, true);
}

string *string_from_filename(const char *filename, size_t *read){
    FILE *file = fopen(filename, "r+b");
    if(file == NULL) return NULL;

    return string_from_file(file, read);
}

string_ite_t string_split(const string *str, const char *tokens){
	return (string_ite_t){.state = str->raw, .tokens = tokens};
}

string *string_next(string_ite_t *iterator){

	size_t start = strspn(iterator->state, iterator->tokens);
	size_t end = strcspn(iterator->state + start, iterator->tokens);
	if(end == 0) return NULL;

	char buffer[STRING_SPLIT_MAX_SIZE] = {0};
	memcpy(buffer, iterator->state + start, end);

	string *split = string_from(buffer);

	iterator->state += start + end;
	return split; 
}

size_t string_length(const string* str){
	return str->len;
}

string *string_slice(const string *str, size_t from, size_t len){
	if(str == NULL || (from + len) > str->len) return NULL;

	char *buffer = malloc(sizeof(char) * (len + 1));
	memcpy(buffer, str->raw + from, len);
	buffer[len] = '\0';

	string *ret = string_from(buffer);
	free(buffer);

	return ret;
}

int64_t string_to_int(const string *str, uint32_t base){
	if(str == NULL) return 0;

	return strtol(str->raw, NULL, base);
}

uint64_t string_to_uint(const string *str, uint32_t base){
	if(str == NULL) return 0;

	return strtoul(str->raw, NULL, base);
}

double string_to_double(const string *str){
	if(str == NULL) return 0;

	return strtod(str->raw, NULL);
}