#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include "err.h"
#include "comms.h"
#include "common.h"
#include "playerCommon.h"
#include "token.h"
#include "card.h"

/* checks if tokens are valid
 * if valid, takes them in the given order
 * params:  tokens - array of available tokens
 *          tokenOrder - order in which to prefer tokens
 * returns: NULL if no tokens taken,
 *          otherwise, an array of which tokens were taken
 */
int* get_tokens(int* tokens, int* tokenOrder) {
#ifdef VERBOSE 
    fprintf(stderr, "got token order:%d,%d,%d,%d\n", 
            tokenOrder[0], tokenOrder[1], tokenOrder[2], tokenOrder[3]);
#endif

    int availableTokens = 0;
    for(int i = 0; i < TOKEN_SIZE; i++) {
        if(tokens[i] > 0) {
            availableTokens++;
        }
    }

    if(availableTokens < 3) {
        return NULL;
    }

    int tokenCount = 0;
    int* currentTokens = (int*)malloc(TOKEN_SIZE * sizeof(int));
    memcpy(currentTokens, tokens, TOKEN_SIZE * sizeof(int));
    int* takenTokens = (int*)calloc(TOKEN_SIZE, sizeof(int));
    for(int i = 0; i < TOKEN_SIZE && tokenCount < 3; i++) {
        if(currentTokens[tokenOrder[i]] > 0) {
            currentTokens[tokenOrder[i]]--;
            tokenCount++;
            takenTokens[tokenOrder[i]] = 1;
        } else {
            takenTokens[tokenOrder[i]] = 0;
        }
    }
    free(currentTokens);

    return takenTokens;
    
}

/*
 * sets the current values of the tokens after being taken
 * params:  game - struct containing relevang game information
 *          card - values to update tokens to
 *          opponents - array of structs containing player information
 *          player - player who took tokens
 * returns: E_COMMERR if invalid token number,
 *          OK otherwise
 */
Error took_tokens(Game* game, Card card, Opponent* opponents, char player) {
    if(card[PURPLE] < 0 || card[PURPLE] > INT_MAX || 
            card[BROWN] < 0 || card[BROWN] > INT_MAX || 
            card[YELLOW] < 0 || card[YELLOW] > INT_MAX || 
            card[RED] < 0 || card[RED] > INT_MAX) {
        return E_COMMERR;
    }

    for(int i = 0; i < TOKEN_SIZE; i++) {
        game->tokens[i] -= card[i + 2];
        opponents[(int)(player - TOCHAR)].tokens[i] += card[i + 2]; 
        if(player == game->pID + TOCHAR) {
            game->ownedTokens[i] += card[i + 2];
        }
    }

#ifdef TEST
    fprintf(stderr, "tokens set: %d,%d,%d,%d; ", 
            game->tokens[0], game->tokens[1],
            game->tokens[2], game->tokens[3]);
    fprintf(stderr, "player %c set: %d,%d,%d,%d\n", player, 
            opponents[(int)(player - TOCHAR)].tokens[0], 
            opponents[(int)(player - TOCHAR)].tokens[1],
            opponents[(int)(player - TOCHAR)].tokens[2], 
            opponents[(int)(player - TOCHAR)].tokens[3]);
#endif
    
    return OK;
}

/*
 * sets the current values of the tokens after being returned
 * params:  game - struct containing relevang game information
 *          card - values to update tokens to
 *          wild - wild tokens returned
 *          opponents - array of structs containing player information
 *          player - player who took tokens
 * returns: E_COMMERR if invalid token number,
 *          OK otherwise
 */
Error returned_tokens(Game* game, Card card, int wild, Opponent* opponents, 
        char player) {
    if(card[PURPLE] < 0 || card[PURPLE] > INT_MAX || 
            card[BROWN] < 0 || card[BROWN] > INT_MAX || 
            card[YELLOW] < 0 || card[YELLOW] > INT_MAX || 
            card[RED] < 0 || card[RED] > INT_MAX) {
        return E_COMMERR;
    }

    for(int i = 0; i < TOKEN_SIZE; i++) {
        game->tokens[i] += card[i + 2];
        opponents[(int)(player - TOCHAR)].tokens[i] -= card[i + 2]; 
        if(player == game->pID + TOCHAR) {
            game->ownedTokens[i] -= card[i + 2];
        }
    }
    
    if(player == game->pID + TOCHAR) {
        game->wild -= wild;
    }

    return OK;
}

/*
 * updates player information with new wild token
 * params:  game - struct containing relevang game information
 *          opponents - array of structs containing player information
 *          player - player who took wild
 */
void update_wild(Game* game, Opponent* opponents, char player) {
    opponents[(int)(player - TOCHAR)].wild += 1;
    if(player == game->pID + TOCHAR) {
        game->wild += 1;
    }
}

/*
 * sets the initial values of the tokens
 * params:  game - struct containing relevang game information
 *          numTokens - number to set tokens to
 * returns: E_COMMERR if invalid token number,
 *          OK otherwise
 */
Error set_tokens(Game* game, int numTokens) {
    if(numTokens < 0 || numTokens > INT_MAX) {
        return E_COMMERR;
    }

    for(int i = 0; i < TOKEN_SIZE; i++) {
        game->tokens[i] = numTokens;
    }

#ifdef TEST
    fprintf(stderr, "tokens set: %d,%d,%d,%d\n", 
            game->tokens[0], game->tokens[1],
            game->tokens[2], game->tokens[3]);
#endif

    return OK;
}

/*
 * checks if player can afford the card with their owned tokens
 * params:  card - card to purchase
 *          discount - discount from owned cards
 *          tokens - players owned tokens
 *          wild - number of owned wild tokens
 * returns: -1 if cannot afford,
 *          number of wild tokens used otherwise
 */
int can_afford(Card card, int* discount, int* tokens, int wild) {
    int usedWild = 0;
    for(int i = 0; i < TOKEN_SIZE; i++) {
        int cost = card[i + 2] - discount[i];
        if(cost > tokens[i] + wild - usedWild) {
            return -1;
        }
        
        if(cost > tokens[i]) {
            usedWild += cost - tokens[i];
        }
    }
        
    return usedWild;
}

/*
 * determines how many tokens are used in the purchase
 * params:  discount - array of discounts from owned cards
 *          tokens - number of owned tokens
 *          card - card to check with
 * returns: int array of tokens used
 */
int* get_card_cost(int* discount, int* tokens, Card card) {
    int* usedTokens = (int*)calloc(TOKEN_SIZE, sizeof(int));
    for(int i = 0; i < TOKEN_SIZE; i++) {
        int cost = card[i + 2] - discount[i];
        if(cost < 1) { // no tokens required
            usedTokens[i] = 0;
        } else { // if tokens required
            if(cost <= tokens[i]) {
                usedTokens[i] = cost;
            } else { // wild tokens required
                usedTokens[i] = tokens[i];
            }
        }

#ifdef TEST
        fprintf(stderr, "cost:%d, discount:%d, have:%d, used:%d, wild:%d\n", 
                card[i + 2] - discount[i], 
                discount[i],
                tokens[i],
                usedTokens[i], 
                card[i + 2] - discount[i] - tokens[i]);
#endif
    }

    return usedTokens;
}

/*
 * adds up the token cost of a card
 * params:  card - struct containing card details
 *          discount - players discount (if any)
 * returns: sum of token costs of a card
 */
int sum_tokens(Card card, int* discount) {
    int total = 0;
    for(int i = 0; i < TOKEN_SIZE; i++) {
        if(card[i + 2]) {
            total += card[i + 2] - discount[i];

#ifdef TEST
            if(discount[i]) {
                fprintf(stderr, "discount[%d] of %d applied\n", 
                        i, discount[i]);
            }
#endif
        }

    }
    
    return total;
}
