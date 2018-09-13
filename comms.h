#ifndef COMMS_H
#define COMMS_H

#include "card.h"

typedef enum {
    EOG,
    DOWHAT,
    TOKENS,
    NEWCARD,
    PURCHASED,
    TOOK,
    PURCHASE,
    TAKE,
    WILD
} Comm;

// message between hub and player, 
// message will be encoded/decoded depending on type
typedef struct {
    Comm type;
    char player;
    int tokens;
    Card info;
    int wild;
    int card;
} Msg;

int send_msg(Msg* msg, int destination);

char* encode_hub(Msg* msg);

char* encode_player(Msg* msg);

Comm decode_hub_msg(Msg* msg, char* input);

Comm decode_player_msg(Msg* msg, char* input);

#endif
