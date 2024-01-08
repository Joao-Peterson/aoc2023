#include "string+.h"
#include "regex.h"
#include "ctype.h"

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

void string_copy_to(string *to, const string *from, size_t len){
	if(
		to == NULL ||
		from == NULL ||
		(to->allocated - 1) < len ||
		(from->allocated - 1) < len
	){
		return;
	}

	memcpy(to->raw, from->raw, len);
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
		strncat(dest->raw, src, srclen);
	}
}

void string_cat_raw(string *dest, const char *src, size_t len){
	if(dest == NULL || src == NULL) return;

	size_t l = strlen(src);
	if(len > 0 && len <= l)
		l = len;

	_string_cat_raw(dest, src, l);
}

void string_cat(string *dest, const string *src, size_t len){
	if(dest == NULL || src == NULL) return;

	size_t l = src->len;
	if(len > 0 && len <= l)
		l = len;

	_string_cat_raw(dest, src->raw, l);
}

void string_vwrite(string *str, const char *fmt, size_t buffer_size, va_list args){
	if(str == NULL || buffer_size == 0) return;

	string *expanded = string_vsprint(fmt, buffer_size, args);
	string_cat(str, expanded, 0);
	string_destroy(expanded);
}

void string_vwriteLn(string *str, const char *fmt, size_t buffer_size, va_list args){
	if(str == NULL || buffer_size == 0) return;
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

string *string_slice(const string *str, int from, unsigned int len){
	if(str == NULL) return NULL;

	if(from < 0) from = str->len + from;
	if(len == 0) len = str->len - from;
	
	if((from + len) > str->len) return NULL;

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

void strings_destroy(strings_t strings){
	for(size_t i = 0; i < strings.size; i++)
		string_destroy(strings.string[i]);
}

typedef struct{
	string *replaced;
	string **matches;
	size_t matchesSize;
}matches_and_replaced_t;

matches_and_replaced_t _string_match_replace_all(const string *str, const char *regex, const char *replace, size_t max, bool storeMatches, bool processReplace, error_t *error){
	regex_t reg;
	int res;
	error_t err = {
		.code = 0,
		.msg = "ok"
	};

	res = regcomp(&reg, regex, REG_EXTENDED | REG_NEWLINE);
	if(res){
		err.code = res;
		regerror(res, &reg, (char*)err.msg, 300);
		if(error != NULL) *error = err; 
		matches_and_replaced_t ret = {0};
		return ret;
	}

	regmatch_t groups[100];

	matches_and_replaced_t ret = {
		.replaced = processReplace ? string_new_sized(str->len) : NULL,
		.matches = storeMatches ? calloc(max, sizeof(string*)) : NULL,
		.matchesSize = 0
	};

	// match again and again 
	char *cursor = str->raw;
	for(size_t m = 0; (max == 0 || m < max) && *cursor != '\0'; m++){
        if(regexec(&reg, cursor, 100, groups, 0)){
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
			ret.matches[ret.matchesSize] = string_new_sized(groups[0].rm_eo - groups[0].rm_so + 1);
			string_cat_raw(ret.matches[ret.matchesSize], cursor + groups[0].rm_so, groups[0].rm_eo - groups[0].rm_so);
			ret.matchesSize++;
		}

		// after end of last match
		cursor = cursor + groups[0].rm_eo;
    }

	// regfree(&reg);
	if(error != NULL) *error = err;
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

strings_t string_matchAll(const string *str, const char *regex, size_t maxMatches, error_t *error){
	matches_and_replaced_t mr = _string_match_replace_all(str, regex, NULL, maxMatches, true, false, error);

	strings_t s = {
		.string = mr.matches,
		.size = mr.matchesSize
	};

	return s;
}

string *string_match(const string *str, const char *regex, error_t *error){
	matches_and_replaced_t mr = _string_match_replace_all(str, regex, NULL, 1, true, false, error);
	return mr.matches[0];
}