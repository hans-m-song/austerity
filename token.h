#ifndef TOKEN_H
#define TOKEN_H

#include "err.h"
#include "comms.h"
#include "card.h"
#include "playerCommon.h"

int sum_tokens(Card card);

int* get_tokens(int* tokens, int* tokenOrder);

Error took_tokens(Game* game, Card card, Opponent* opponents, char player);

Error returned_tokens(Game* game, Card card, int wild, Opponent* opponents, 
        char player);

void update_wild(Game* game, Opponent* opponents, char player);

Error set_tokens(Game* game, int numTokens);

int can_afford(Card card, int* discount, int* tokens, int wild);

int sum_tokens(Card card);

int* get_card_cost(int* discount, int* tokens, Card card);

#endif
