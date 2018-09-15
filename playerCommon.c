#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "err.h"
#include "common.h"
#include "playerCommon.h"
#include "comms.h"

/*
 * checks if the given string is valid
 * params:  input - string to check
 * returns: ERR if string is invalid,
 *          otherwise returns the number of players
 */
int check_pcount(char* input) {
    char* temp;
    long int pCount = strtol(input, &temp, 10);
    if(pCount < 2 || pCount > 26) {
        return ERR;
    }
    return (int)pCount;
}

/*
 * checks if the given string is valid
 * params:  input - string to check
 * returns: ERR if string is invalid,
 *          otherwise returns the number of players
 */
int check_pid(char* input, int pCount) {
    char* temp;
    long int pID = strtol(input, &temp, 10);
    if(pID < 0 || pID >= pCount) {
        return ERR;
    }
    return (int)pID;
}

/*
 * initializes game for players
 * params:  pID - id of player from 0 to pCount
 *          pCount - number of players in game
 *          game - struct containing game relevant information
 */
void init_player_game(int pID, int pCount, Game* game) {
    game->pID = pID;
    game->pCount = pCount;
    game->numPoints = 0;
    game->stack.numCards = 0;
    game->ownedCards.numCards = 0;
    memset(game->tokens, 0, sizeof(int) * TOKEN_SIZE);
    game->wild = 0;
}

/*
 * TODO send_move? encodes a message and sends it to the given destination
 * params:  msg - message to encode
 *          destination - where to send the message to
 */
void send_move(Msg* msg) {
    char* encodedMsg = encode_player(msg);
    printf("%s\n", encodedMsg);
    free(msg);
    free(encodedMsg);
}

/*
 * adds the information about the card from the message to the stack
 * params:  game - struct containing relevang game information
 *          msg - struct containing information from the hub
 * returns: ERR if addcard fails,
 *          OK otherwise
 */
Error newcard(Game* game, Msg* msg) {
    return add_card(&game->stack, msg->info[COLOR], msg->info[POINTS], 
            msg->info[PURPLE], msg->info[BROWN], 
            msg->info[YELLOW], msg->info[RED]);
}

/*
 * sets the initial values of the tokens
 * params:  game - struct containing relevang game information
 *          numTokens - number to set tokens to
 * returns: Err if invalid token number,
 *          OK otherwise
 */
Error set_tokens(Game* game, int numTokens) {
    if(numTokens < 0 || numTokens > INT_MAX) {
        return ERR;
    }

    for(int i = 0; i < TOKEN_SIZE; i++) {
        game->tokens[i] = numTokens;
    }

    return OK;
}

/*
 * sets the current values of the tokens
 * params:  game - struct containing relevang game information
 *          purple, brown, yellow, red - values to update tokens to
 * returns: ERR if invalid token number,
 *          OK otherwise
 */
Error update_tokens(Game* game, Card card) {
    if(card[PURPLE] < 0 || card[PURPLE] > INT_MAX || 
            card[BROWN] < 0 || card[BROWN] > INT_MAX || 
            card[YELLOW] < 0 || card[YELLOW] > INT_MAX || 
            card[RED] < 0 || card[RED] > INT_MAX) {
        return ERR;
    }

    game->tokens[0] = card[PURPLE];
    game->tokens[1] = card[BROWN];
    game->tokens[2] = card[YELLOW];
    game->tokens[3] = card[RED];
    
    return OK;
}

/*
 * main driver for logic of players
 * params:  game - struct containing game relevant information
 *          move - function pointer to the player-specific move logic
 * returns: E_COMMERR if bad message received,
 *          UTIL otherwise for end of game
 */
Error play_game(Game* game, Msg* (*playerMove)(Game*)) {
    Error err = OK;
    //int resetDeckFlag = 0;
    char* line;
    Msg msg; 
    msg.info = (Card)malloc(sizeof(int) * CARD_SIZE);
    while(err == OK) {
        line = read_line(stdin);
        if(!line) {
            break;
        }
            
        if((int)decode_hub_msg(&msg, line) == ERR) {
            break;
        }

        switch(msg.type) {
            case EOG:
                err = UTIL;
                break;
            case DOWHAT:
                send_move(playerMove(game));
                break;
            case TOKENS:
                err = set_tokens(game, msg.tokens);
                break;
            case NEWCARD:
                err = newcard(game, &msg);
                break;
            case PURCHASED:
                err = remove_card(&game->stack, msg.card);
                break;
            case TOOK:
                err = update_tokens(game, msg.info);
                break;
            case WILD:
                break;
            default:
                err = E_COMMERR;
        }
    }
    free(msg.info);
    return err;
}

// TODO get_tokens checks if tokens are valid
// if valid, takes them in the given order
// params:  tokens - array of available tokens
//          tokenOrder - order in which to prefer tokens
// returns: NULL if no tokens taken,
//          otherwise, an array of which tokens were taken
int* get_tokens(int tokens[TOKEN_SIZE], int tokenOrder[TOKEN_SIZE]) {
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
    int* takenTokens = (int*)malloc(sizeof(int) * TOKEN_SIZE);
    for(int i = 0; i < TOKEN_SIZE && tokenCount < 3; i++) {
        // TODO take tokens
        if(takenTokens[tokenOrder[i]] > 0) {
            takenTokens[tokenOrder[i]]--;
            tokenCount++;
            takenTokens[tokenOrder[i]] = 1;
        } else {
            takenTokens[tokenOrder[i]] = 0;
        }
    }

    return takenTokens;
    
}

/*
 * adds up the token cost of a card
 * params:  card - struct containing card details
 * returns: sum of token costs of a card
 */
int sum_tokens(Card card) {
    int total = 0;
    for(int i = PURPLE; i < CARD_SIZE; i++) {
        total += card[i];
    }
    return total;
}

/*
 * checks if player can afford the card with their owned tokens
 * params:  card - card to purchase
 *          tokens - players owned tokens
 *          wild - number of owned wild tokens
 * returns: 0 if cannot afford,
 *          1 otherwise
 */
int can_afford(Card card, int tokens[TOKEN_SIZE], int wild) {
    int usedWild = 0;
    for(int i = 0; i < TOKEN_SIZE; i++) {
        if(card[i + 2] > tokens[i] + wild - usedWild) {
            return 0;
        }

        if(card[i + 2] > tokens[i]) {
            usedWild += card[i + 2] - tokens[i];
            tokens[i] = 0;
        } else {
            tokens[i] -= card[i + 2];
        }
    }
    
    return 1;
}
