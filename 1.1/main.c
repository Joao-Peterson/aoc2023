#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

void *filetomem(char *filename, size_t *filesize);
uint64_t sumCals(char * calibrations);

int main(int argc, char**argv){
	char *data;
	
	// from arg filename
	if(argc > 1){
		data = filetomem(argv[1], NULL);
	}
	// from pipe
	else{
		data = malloc(1024*1000);
		char buffer[1024];
		*data = '\0';
		*buffer = '\0';
		while(fgets(buffer, 1024, stdin) != NULL){
			strcat(data, buffer);
		}
	}
	
	printf("%u", sumCals(data));
	
	free(data);

	return 0;
}

uint64_t sumCals(char * calibrations){
	int first = -1;
	int last = -1;
	uint16_t acc = 0;

	for(char *cursor = calibrations; *cursor != '\0'; cursor++){
		if(isdigit(*cursor)){
			last = (int)*cursor;
			if(first == -1) 
				first = last;
		}
		
		if(*cursor == '\n'){
			acc += (first - '0')*10 + (last - '0');  
			first = -1;
			last = -1;
		}
	}

	return acc;
}

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
