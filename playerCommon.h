#ifndef PCOMMON_H
#define PCOMMON_H

#include "common.h"
#include "comms.h"

#define SHENZI 0
#define BANZAI 1
#define ED 2

typedef struct {
    int id;
    int numPoints;
    int discount[4];
    int tokens[4];
    int wild;
} Opponent;

int check_pcount(char* input);

int check_pid(char* input, int pCount);

void init_player_game(int pID, int pCount, Game* game);

void player_status(Comm type, char* winners);

Error play_game(Game* game, Msg* (*playerMove)(Game*));

int sum_tokens(Card card);

int* get_tokens(int* tokens, int tokenOrder[TOKEN_SIZE]);

int can_afford(Card card, int ownedTokens[TOKEN_SIZE], int wild);

#endif
