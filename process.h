#ifndef PROCESS_H
#define PROCESS_H

#include <sys/types.h>
#include "err.h"
#include "common.h"

// information used by hub for communication to players
typedef struct {
    pid_t pid;
    int pipeIn[2];
    int pipeOut[2];
    FILE* toChild;
    FILE* fromChild;
} Player;

// hub only information, contains player communication,
// player stats and the parentPID for identification
typedef struct {
    int id;
    pid_t parentPID;
    Player* players;
    Game* playerStats;
} Session;

void kill_players(int pCount, Player* players, Error err);

Error start_players(int pCount, char** players, Game* game, Session* session);

#endif
