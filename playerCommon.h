#ifndef PCOMMON_H
#define PCOMMON_H

#include "common.h"
#include "comms.h"

#define SHENZI 0
#define BANZAI 1
#define ED 2

// struct used by players to keep track of opponents
typedef struct {
    int id;
    int numPoints;
    int discount[4];
    int tokens[4];
    int wild;
} Opponent;

int check_pcount(char* input);

int check_pid(char* input, int pCount);

int* sort_by_points(int numCards, Deck deck);

void init_player_game(int pID, int pCount, Game* game);

void player_status(Comm type, char* winners);

Error play_game(Game* game, Msg* (*playerMove)(Game*, ...));

Error play_ed_game(Game* game, Msg* (*playerMove)(Game*, Opponent*, int));

int* get_tokens(int* tokens, int tokenOrder[TOKEN_SIZE]);

#endif
