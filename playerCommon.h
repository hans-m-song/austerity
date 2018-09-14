#ifndef PCOMMON_H
#define PCOMMON_H

#include "common.h"
#include "comms.h"

#define SHENZI 0
#define BANZAI 1
#define ED 2

int check_pcount(char* input);

int check_pid(char* input, int pCount);

void init_player_game(int pID, int pCount, Game* game);

void player_status(Comm type, char* winners);

Error play_game(Game* game, Msg* (*playerMove)(Game*));

int* get_tokens(int tokens[TOKEN_SIZE], int tokenOrder[TOKEN_SIZE]);

#endif
