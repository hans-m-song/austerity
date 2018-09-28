#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include "card.h"

// pipe size
#define FD_SIZE 2
// pipe stream directions
#define READ 0
#define WRITE 1
// maximum buffer size
#define LINE_BUFF 50
// number of types of tokens
#define TOKEN_SIZE 4
// maximum number of players
#define MAX_PLAYERS 26
// converting int to char (e.g. 0 -> 'A')
#define TOCHAR 65

// struct containing information relevant to the game
// not all fields will be used by hub or player
// e.g. pID is irrelevant for hub, hubStack will hold cards not in the game,
// numPoints will either be the winning amount or the amount currently owned
typedef struct {
    int pID;
    int pCount;
    int numPoints;
    Stack stack;
    Stack hubStack;
    int discount[TOKEN_SIZE];
    int tokens[TOKEN_SIZE];
    int ownedTokens[TOKEN_SIZE];
    int wild;
} Game;

char* to_string(int input);

char* read_line(FILE* input, int space, int newline);

void concat(char* input1, char* input2);

void charcat(char* input1, int position, char input2);

int has_element(int* array, int len, int element);

int is_all_num(char* input);

#endif

