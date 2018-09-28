#ifndef HUB_INIT
#define HUB_INIT

#include <stdlib.h>
#include <sys/types.h>
#include "common.h"
#include "comms.h"
#include "process.h"

Error hub_init(char** argv, Game* game);

Error broadcast(int pCount, Player* players, Msg* msg);

Error send_msg(Msg* msg, FILE* destination);

#endif
