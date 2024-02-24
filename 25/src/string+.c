#include "string+.h"
#include "regex.h"
#include "ctype.h"
#include <errno.h>

/**
 * !Obs:
 * !trivial marked functions contain low level char access, therefore should be refactored carefully,
 * as the other functions use these trivial ones to build upon  
*/

// ------------------------------------------------------------ String -------------------------------------------------------------

// !trivial
string *string_new_sized(size_t size){
	string *str = calloc(sizeof(char), sizeof(string));
	str->len = 0;
	str->allocated = size;
	str->owns = true;

	if(str->allocated > 0 )
		str->raw = calloc(str->allocated, sizeof(char));
	else
		str->raw = NULL;

	return str;
}

string *string_new(){
	return string_new_sized(STRING_ALLOCATION_CHUNK);
}

// !trivial
string *string_copy(const string *str){
	if(str == NULL) return NULL;
	string *new = string_new_sized(str->allocated);
	new->len = str->len;
	memcpy(new->raw, str->raw, str->len);
	return new;
}

// !trivial
string *string_from(const char *raw){
	if(raw == NULL) return NULL;
	size_t len = strlen(raw);
	string *str = string_new_sized(len + 1);
	str->len = len;
	memcpy(str->raw, raw, len);
	return str;
}

// !trivial
string *string_wrap(char *raw, bool take_ownership){
	if(raw == NULL) return NULL;
	size_t len = strlen(raw);
	string *str = malloc(sizeof(string));
	str->allocated = 0;
	str->owns = take_ownership;
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
	if(str->owns)
		free(str->raw);

	free(str);
}

char *string_unwrap(string *str){
	char *raw = str->raw;
	free(str);
	return raw;
}

// !trivial
void string_println(const string *str){
	printf("%.*s\n", (int)str->len, str->raw);
}

// !trivial
void string_print(const string *str){
	printf("%.*s", (int)str->len, str->raw);
}

// !trivial
size_t string_write_to_file(const string *str, FILE *file){
	if(file == NULL) return 0;
	return fwrite(str->raw, sizeof(char), str->len, file);
}

error_t string_save_to_file(const string *str, const char *filename){
	FILE *f = fopen(filename, "w");

	if(f == NULL){
		error_t ret = (error_t){0};
		ret.code = errno;
		const char *err = strerror(errno);
		strncat(ret.msg, err, 300 - 1);
		return ret;
	}

	fwrite(str->raw, sizeof(char), str->len, f);
	fflush(f);
	fclose(f);
	return (error_t){.code = 0, .msg = "ok"};
}

// !trivial
bool string_cmp_raw(const string *a, const char *b){
	if(b == NULL) return false;
	size_t max = strlen(b);
	if(a->len != max) return false;

	for(size_t i = 0; i < max; i++){
		if(a->raw[i] != b[i])
			return false;
	}

	return true;
}

bool string_cmp(const string *a, const string *b){
	return string_cmp_raw(a, b->raw);
}

// !trivial
void _string_cat_raw(string * restrict dest, const char * restrict src, size_t srclen){
	// no src or dest is string slice
	if(src == NULL || (dest->allocated == 0 && dest->owns == false)) return;

	size_t len = dest->len + srclen;
	if(dest->allocated == 0){
		dest->allocated = len + STRING_ALLOCATION_CHUNK;
		dest->raw = calloc(dest->allocated, sizeof(char));
		dest->len = len;
		memcpy(dest->raw, src, len);	
	}
	else{
		if(dest->allocated < (len + 1)){
			dest->allocated = len + STRING_ALLOCATION_CHUNK; // add more just in case
			dest->raw = realloc(dest->raw, dest->allocated);
		}

		dest->len = len;
		strncat(dest->raw, src, srclen);
	}
}

void string_cat_raw(string * restrict dest, const char * restrict src, size_t len){
	if(src == NULL) return;

	size_t l = strlen(src);
	if(len > 0 && len <= l)
		l = len;

	_string_cat_raw(dest, src, l);
}

void string_cat(string * restrict dest, const string * restrict src, size_t len){
	size_t l = src->len;
	if(len > 0 && len <= l)
		l = len;

	_string_cat_raw(dest, src->raw, l);
}

void string_vwrite(string *str, const char *fmt, size_t buffer_size, va_list args){
	if(buffer_size == 0) return;
	string *expanded = string_vsprint(fmt, buffer_size, args);
	string_cat(str, expanded, 0);
	string_destroy(expanded);
}

void string_vwriteLn(string *str, const char *fmt, size_t buffer_size, va_list args){
	if(buffer_size == 0) return;
	string_vwrite(str, fmt, buffer_size, args);
	string_cat_raw(str, "\n", 0);
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
	string_cat_raw(str, "\n", 0);
	va_end(args);
}

string *string_from_file(FILE *file, size_t *read){
    if(file == NULL) return NULL;

    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *buffer = (char*)malloc(size + 1);
    fread(buffer, size, 1, file);

    fclose(file);

    buffer[size] = 0;

    if(read != NULL)
        *read = size;

    return string_wrap(buffer, true);
}

string *string_from_filename(const char *filename, size_t *read){
    FILE *file = fopen(filename, "r+b");
    if(file == NULL) return NULL;

    return string_from_file(file, read);
}

// !trivial
string string_ite_next(string_ite *iterator){
	size_t start = strspn(iterator->state, iterator->tokens);
	size_t end = strcspn(iterator->state + start, iterator->tokens);

	// no more string
	if(end == 0 || iterator->state + start >= iterator->base + iterator->max){
		iterator->yield = false;
		return (string){0};
	}

	// string ended
	if(iterator->state + start + end > iterator->base + iterator->max){
		// correct end to the base max
		end = iterator->base + iterator->max - (iterator->state + start);
	}
	
	string slice = (string){
		.raw = iterator->state + start,
		.len = end,
		.allocated = 0,
		.owns = false
	};

	iterator->state += start + end;
	return slice; 
}

string_ite string_split(const string *str, const char *tokens){
	return (string_ite){
		.state = str->raw,
		.base = str->raw,
		.max = str->len,
		.tokens = tokens,
		.next = string_ite_next,
		.yield = true
	};
}

// !trivial
string string_slice(const string *str, int from, unsigned int len){
	// negative is from the end coming left
	if(from < 0) from = str->len + from;
	// 0 is all len remaining
	if(len == 0) len = str->len - from;
	// if bigger, just use all len
	if((from + len) > str->len) len = str->len - from;

	return (string){
		.allocated = 0,
		.owns = false,
		.len = len,
		.raw = str->raw + from
	};
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

int getGroupId(char *replaceExpression, char **groupIdStart, char** groupIdEnd){
	for(char *cursor = replaceExpression; cursor != NULL && *cursor != '\0'; cursor++){
		if(cursor[0] == '&' && isdigit(cursor[1])){
			*groupIdStart = cursor;
			cursor++;
			char *end;
			int id = strtod(cursor, &end);
			*groupIdEnd = end;
			return id;
		}
	}

	*groupIdStart = NULL;
	*groupIdEnd = NULL;
	return -1;
}

typedef struct{
	string *replaced;
	string **matches;
	size_t matchesSize;
}matches_and_replaced_t;

// !trivial
matches_and_replaced_t _string_match_replace_all(const string *str, const char *regex, const char *replace, size_t max, bool storeMatches, bool processReplace, error_t *err){
	regex_t reg;
	int res;

	// reg compile
	if((res = regcomp(&reg, regex, REG_EXTENDED | REG_NEWLINE))){
		if(err != NULL){
			err->code = res;
			regerror(res, &reg, (char*)err->msg, 300);
		} 
		return (matches_and_replaced_t){0};
	}

	regmatch_t groups[100];

	matches_and_replaced_t ret = {
		.replaced = processReplace ? string_new_sized(str->len + STRING_ALLOCATION_CHUNK) : NULL,
		.matches = storeMatches ? calloc(max, sizeof(string*)) : NULL,
		.matchesSize = 0
	};

	// match again and again 
	char *cursor = str->raw;
	for(size_t m = 0; (max == 0 || m < max) && (cursor < (str->raw + str->len)); m++){
        if((res = regexec(&reg, cursor, 100, groups, 0))){
			if(err != NULL){
				err->code = res;
				regerror(res, &reg, (char*)err->msg, 300);
			} 
			
			// append rest on exit
			if(processReplace)
				string_cat_raw(ret.replaced, cursor, 0);

			break;
		}

		// groups
		int groupSize = 0;
		for(size_t i = 0; i < 100; i++){
			if(groups[i].rm_so == -1) break;
			
			// correct offsets
			// groups[i].rm_so += cursor - str->raw;
			// groups[i].rm_eo += cursor - str->raw;

			groupSize++;	
		}

		if(processReplace){
			// add original string
			string_cat_raw(ret.replaced, cursor, groups[0].rm_so);

			// substitute groups
			char *replaceCursor = (char*)replace;
			char *endText;
			char *end;
			int id = getGroupId(replaceCursor, &endText, &end);
			while(replaceCursor != NULL){
				string_cat_raw(ret.replaced, (const char *)replaceCursor, endText - replaceCursor);

				if(id > -1 && id < groupSize)
					string_cat_raw(ret.replaced, cursor + groups[id].rm_so, groups[id].rm_eo - groups[id].rm_so);

				replaceCursor = end;
				id = getGroupId(replaceCursor, &endText, &end);
			}
		}

		if(storeMatches){
			// store matched
			ret.matches[ret.matchesSize] = string_wrap(cursor + groups[0].rm_so, false);
			ret.matches[ret.matchesSize]->len = groups[0].rm_eo - groups[0].rm_so;
			ret.matchesSize++;
		}

		// after end of last match
		cursor = cursor + groups[0].rm_eo;
    }

	// regfree(&reg);
	if(err != NULL) *err = (error_t){.code = 0, .msg = "ok"};
	return ret;
}

string *string_replaceAll(const string *str, const char *regex, const char *replace, size_t maxReplaces, error_t *error){
	matches_and_replaced_t mr = _string_match_replace_all(str, regex, replace, maxReplaces, false, true, error);
	return mr.replaced;
}

string *string_replace(const string *str, const char *regex, const char *replace, error_t *error){
	matches_and_replaced_t mr = _string_match_replace_all(str, regex, replace, 1, false, true, error);
	return mr.replaced;
}

array_t *string_matchAll(const string *str, const char *regex, size_t maxMatches, error_t *error){
	matches_and_replaced_t mr = _string_match_replace_all(str, regex, NULL, maxMatches, true, false, error);

	array_t *ret = array_new_custom(true, mr.matchesSize);
	for(size_t i = 0; i < mr.matchesSize; i++)
		array_set(ret, i, mr.matches[i]);

	free(mr.matches);
	return ret;
}

string string_match(const string *str, const char *regex, error_t *error){
	matches_and_replaced_t mr = _string_match_replace_all(str, regex, NULL, 1, true, false, error);
	string ret = *mr.matches[0];
	free(mr.matches);
	return ret;
}