#ifndef COMMON_H
#define COMMON_H

#include "card.h"

#define IN 0
#define OUT 1
#define LINE_BUFF 20
#define NAME_SIZE 10

// struct containing information relevant to the game
typedef struct {
    int pCount;
    int numTokens;
    int numPoints;
    int numCards;
    Stack stack;
} Game;

char* to_string(int input);

#endif

