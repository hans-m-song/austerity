#ifndef PROCESS_H
#define PROCESS_H

#include <sys/types.h>
#include "err.h"
#include "common.h"

typedef struct {
    pid_t pid;
    int pipeIn[2];
    int pipeOut[2];
    int controlPipe[2];
} Player;

typedef struct {
    int id;
    pid_t parentPID;
    Player* players;
} Session;

void kill_players(int pCount, Player* players, Error err);

Error start_players(int pCount, char** players, Game* game, Session* session);

#endif
