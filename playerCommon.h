#ifndef PCOMMON_H
#define PCOMMON_H

#include "common.h"

int check_pcount(char* input);

int check_pid(char* input, int pCount);

void init_player_game(int pID, int pCount, Game* game);

#endif
