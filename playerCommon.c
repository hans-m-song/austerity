#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <signal.h>
#include "err.h"
#include "common.h"
#include "playerCommon.h"
#include "comms.h"
#include "signalHandler.h"
#include "token.h"

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
    memset(game->discount, 0, sizeof(int) * TOKEN_SIZE);
    memset(game->tokens, 0, sizeof(int) * TOKEN_SIZE);
    memset(game->ownedTokens, 0, sizeof(int) * TOKEN_SIZE);
    game->wild = 0;
}

/*
 * initalizes local record of opponents stats
 * params:  pCount - number of players
 * returns: array of structs containing player stats
 */
Opponent* init_opponents(int pCount) {
    Opponent* opponents = (Opponent*)malloc(sizeof(Opponent) * pCount);
    
    for(int i = 0; i < pCount; i++) {
        opponents[i].id = i;
        opponents[i].numPoints = 0;
        memset(opponents[i].discount, 0, sizeof(int) * TOKEN_SIZE);
        memset(opponents[i].tokens, 0, sizeof(int) * TOKEN_SIZE);
        opponents[i].wild = 0;
    }

    return opponents;
}

/*
 * prints the winning players
 * params:  pCount - number of player in the game
 *          opponents - array of structs containing player information
 * returns: UTIL to indicate end of game
 */
Error print_winners(int pCount, Opponent* opponents) {
    fprintf(stderr, "Game over. Winners are ");
    int max = 0;
    for(int i = 0; i < pCount; i++) {
        if(max < opponents[i].numPoints) {
            max = opponents[i].numPoints;
        }
    }

    // TODO test winning players
    int count = 0;
    for(int i = 0; i < pCount; i++) {
        if(opponents[i].numPoints == max) {
            count++;
        }
    }

    for(int i = 0; i < pCount; i++) {
        if(opponents[i].numPoints == max) {
            fprintf(stderr, "%c", (char)(i + TOCHAR));
            if(--count) {
                fprintf(stderr, ",");
            }
        }
    }

    fprintf(stderr, "\n");

    return UTIL;
}

/*
 * encodes a message and sends it to the given destination
 * params:  game - struct containing relevang game information
 *          msg - message to encode
 * returns: E_COMMERR if broken pipe or invalid message,
 *          OK otherwise
 */
Error send_move(Game* game, Msg* msg) {
    fprintf(stderr, "Received dowhat\n");
    msg->player = game->pID + TOCHAR;    
    
    char* encodedMsg = encode_player(msg);
    if(!encodedMsg) {
        if(msg->type == PURCHASE || msg->type == TAKE) {
            free(msg->info);
        }

        return E_COMMERR;
    }
   
    fprintf(stdout, "%s\n", encodedMsg);
    free(encodedMsg);
    
    if(msg->type == PURCHASE || msg->type == TAKE) {
        free(msg->info);
    }
    free(msg);
    
    if(check_signal()) {
        return E_COMMERR;
    }

    return OK;
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
 * updates player information and deck when card is bought
 * params:  game - struct containing relevang game information
 *          opponents - array of structs containing player information
 *          msg - struct containing message contents
 * returns: E_COMMERR if removecard fails,
 *          OK otherwise
 */
Error bought_card(Game* game, Opponent* opponents, Msg* msg) {
    opponents[(int)(msg->player - TOCHAR)].numPoints += msg->info[POINTS];
    int discountColor = 0;
    switch(game->stack.deck[msg->card][COLOR]) { // do nothing if purple
        case 'B':
            discountColor = 1;
            break;
        case 'Y':
            discountColor = 2;
            break;
        case 'R':
            discountColor = 3;
    }

    opponents[(int)(msg->player - TOCHAR)].discount[discountColor] += 1;
    opponents[(int)(msg->player - TOCHAR)].numPoints += 
            game->stack.deck[msg->card][POINTS];
    opponents[(int)(msg->player - TOCHAR)].wild -= msg->wild;
   
    if(msg->player == game->pID + TOCHAR) {
        game->discount[discountColor] += 1;
        game->numPoints += game->stack.deck[msg->card][POINTS];
    }
    
    Error err = OK;
    
    err = returned_tokens(game, msg->info, msg->wild, opponents, msg->player);
    if(err) {
        return E_COMMERR;
    }
    
    err = remove_card(&game->stack, msg->card);
    if(err) {
        return E_COMMERR;
    }

    if(check_signal()) {
        return E_COMMERR;
    }

    return OK;
}

/*
 * prints the status of every players and the cards in play
 * params:  game - struct containing relevang game information
 *          opponents - array of structs containing player information
 *          int msgType - type of message preceeding this function call
 *          err - error status of game
 */
void print_status(Game* game, Opponent* opponents, int msgType, Error err) {
    if(msgType == DOWHAT || msgType == EOG || err != OK || check_signal()) {
        return;
    }

    print_deck(game->stack.deck, game->stack.numCards);
    for(int i = 0; i < game->pCount; i++) {
        fprintf(stderr, "Player %c:%d:Discounts=%d,%d,%d,%d"
                ":Tokens=%d,%d,%d,%d,%d\n", 
                (char)(opponents[i].id + TOCHAR), opponents[i].numPoints,
                opponents[i].discount[0], opponents[i].discount[1], 
                opponents[i].discount[2], opponents[i].discount[3],
                opponents[i].tokens[0], opponents[i].tokens[1],
                opponents[i].tokens[2], opponents[i].tokens[3],
                opponents[i].wild);
    }
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
    char* line;
    Msg msg;
    msg.info = (Card)malloc(sizeof(int) * CARD_SIZE);
    Opponent* opponents = init_opponents(game->pCount);
    while(err == OK && !check_signal()) {
        line = read_line(stdin);
        if(line == NULL || (int)decode_hub_msg(&msg, line) == ERR ||
                check_signal()) {
            err = E_COMMERR;
            break;
        }

        switch(msg.type) {
            case EOG:
                err = print_winners(game->pCount, opponents);
                break;
            case DOWHAT:
                err = send_move(game, playerMove(game));
                break;
            case TOKENS:
                err = set_tokens(game, msg.tokens);
                break;
            case NEWCARD:
                err = newcard(game, &msg);
                break;
            case PURCHASED:
                err = bought_card(game, opponents, &msg);
                break;
            case TOOK:
                err = took_tokens(game, msg.info, opponents, msg.player);
                break;
            case WILD:
                update_wild(game, opponents, msg.player);
                break;
            default:
                err = E_COMMERR;
        }
        print_status(game, opponents, msg.type, err);
    }
    free(msg.info);
    free(opponents);
    return err;
}
