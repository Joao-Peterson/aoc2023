#ifndef _UTILS_HEADER_
#define _UTILS_HEADER_

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

// read file to single heap string
void *filetomem(char *filename, size_t *filesize){
    FILE *file = fopen(filename, "r+b");
    if(file == NULL) return NULL;

    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    fseek(file, 0, SEEK_SET);

    uint8_t *buffer = malloc(size + 1);
    fread(buffer, size, 1, file);

    fclose(file);

    buffer[size] = 0;

    if(filesize != NULL)
        *filesize = size;
    return buffer;
}

#endif