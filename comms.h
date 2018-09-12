#ifndef COMMS_H
#define COMMS_H

#include "card.h"

typedef enum {
    EOG,
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
    int token;
    Card info;
} Msg;

int encode(Msg* msg);

int decode(Msg* msg);

int send_msg(Msg* msg, int destination);

#endif
