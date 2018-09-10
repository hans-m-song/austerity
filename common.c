#include <stdio.h>
#include <stdlib.h>
#include "common.h"

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
