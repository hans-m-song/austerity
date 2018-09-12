#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "err.h"
#include "playerCommon.h"

/*
 * checks if the given string is valid
 * params:  input - string to check
 * returns: ERR if string is invalid,
 *          otherwise returns the number of players
 */
int check_pcount(char* input) {
    char* temp;
    long int pCount = strtol(input, &temp, 10);
    if(pCount < 2 || pCount > UINT_MAX) {
        return ERR;
    }
    return (int)pCount;
}

/*
 * checks if the given string is valid
 * params:  input - string to check
 * returns: ERR if string is invalid,
 *          otherwise returns the number of players
 */
int check_pid(char* input, int pCount) {
    char* temp;
    long int pID = strtol(input, &temp, 10);
    if(pID < 0 || pID >= pCount) {
        return ERR;
    }
    return (int)pID;
}
