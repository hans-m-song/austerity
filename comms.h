#ifndef COMMS_H
#define COMMS_H

#include "err.h"
#include "card.h"

// message types sent between player and hub
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

// deconstructed message between hub and player, 
// message will be encoded/decoded depending on type,
// not all fields will be used depending on type
typedef struct {
    Comm type;
    char player;
    int tokens;
    Card info;
    int wild;
    int card;
} Msg;

char* encode_hub(Msg* msg);

char* encode_player(Msg* msg);

Comm decode_hub_msg(Msg* msg, char* input);

Comm decode_player_msg(Msg* msg, char* input);

#endif
