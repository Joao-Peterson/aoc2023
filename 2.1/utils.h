#ifndef _UTILS_HEADER_
#define _UTILS_HEADER_

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <regex.h>

// read file to single heap string
void *filetomem(const char *filename, size_t *filesize){
    FILE *file = fopen(filename, "r+b");
    if(file == NULL) return NULL;

    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    fseek(file, 0, SEEK_SET);

    uint8_t *buffer = (uint8_t*)malloc(size + 1);
    fread(buffer, size, 1, file);

    fclose(file);

    buffer[size] = 0;

    if(filesize != NULL)
        *filesize = size;
    return buffer;
}

typedef struct{
    int size;
    regmatch_t matches[100][50];
}regmatches_t;

// return multiple matches rfrom regex
regmatches_t regexecAll(regex_t *regex, char *string){
    regmatches_t matches;
    matches.size = 0;

    for(char *cursor = string; *cursor != '\0'; cursor++){
        if(!regexec(regex, cursor, 50, matches.matches[matches.size], 0)){
            // correct offset
            for(size_t i = 0; i < 50; i++){
                if(matches.matches[matches.size][i].rm_so == -1) break;
                matches.matches[matches.size][i].rm_so += cursor - string;
                matches.matches[matches.size][i].rm_eo += cursor - string;
            }

            cursor = string + matches.matches[matches.size][0].rm_eo;
            matches.size++;

            if(*cursor == '\0') break;
        }
        else{
            break;
        }
    }

    return matches;
}

#endif