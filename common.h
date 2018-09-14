#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include "card.h"

#define IN 0
#define OUT 1
#define LINE_BUFF 50
#define TOKEN_SIZE 4
#define MAX_PLAYERS 26

// struct containing information relevant to the game
// for the hub, pID is unused, numPoints contains the winning number of points
// required and numCards holds the total number of cards, the stack holds
// all cards
// for the player, numTokens and numPoints indicate how many of each is owned 
// by the player, the stack contains the owned cards
typedef struct {
    int pID;
    int pCount;
    int numTokens;
    int numPoints;
    Stack stack;
    int tokens[4];
} Game;

char* to_string(int input);

char* read_line(FILE* input);

#endif

