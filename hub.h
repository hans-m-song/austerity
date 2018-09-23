#ifndef HUB_INIT
#define HUB_INIT

#include <sys/types.h>
#include "common.h"
#include "comms.h"

typedef struct {
    pid_t pid;
    int pipeIn[2];
    int pipeOut[2];
} Player;

typedef struct {
    int id;
    pid_t parentPID;
    Player* players;
} Session;

Error hub_init(char** argv, Game* game);

Error broadcast(int pCount, Player* players, Msg* msg);

#endif
