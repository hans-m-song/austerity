#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "err.h"

/*
 * converts the given integer to a string
 * params:  input - integer to convert
 * returns: output - string representation of input
 */
char* to_string(int input) {
    char* output = (char*)malloc(sizeof(char) * LINE_BUFF);
    snprintf(output, LINE_BUFF, "%d", input);
    return output;
}

/*
 * reads characters from a filestream until EOF or newline is encountered
 * params:  input - filestream to read from
 * returns: string ending with a newline 
 *          (replacing newline with a null terminator),
 *          null if error encountered
 */
char* read_line(FILE* input) {
    int size = LINE_BUFF;
    char* result = (char*)malloc(sizeof(char) * size);
    int pos = 0;
    int i = 0;
    while(1) {
        i = fgetc(input);
        if(i == EOF) {
            free(result);
            return NULL;
        }
        
        if(i == '\n') {
            result[pos] = '\0';
            return result;
        }
        
        result[pos++] = (char)i;

        if(pos > size - 1) {
            size *= 2;
            result = (char*)realloc(result, sizeof(char) * size);
        }
    }
}

/*
 * concatenates the given string to the output string 
 * and frees the added string
 * params:  input1 - string to concatenate to
 *          input2 - string to concatenate with
 */
void concat(char* input1, char* input2) {
    strcat(input1, input2);
    free(input2);
}
