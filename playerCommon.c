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
    game->ownedCards.numCards = 0;
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
        opponents[i].points = 0;
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
        if(max < opponents[i].points) {
            max = opponents[i].points;
        }
    }

    // TODO test winning players
    int count = 0;
    for(int i = 0; i < pCount; i++) {
        if(opponents[i].points == max) {
            count++;
        }
    }

    for(int i = 0; i < pCount; i++) {
        if(opponents[i].points == max) {
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
 *          opponents - array of structs containing player information
 *          msg - message to encode
 * returns: E_COMMERR if broken pipe or invalid message,
 *          OK otherwise
 */
Error send_move(Game* game, Opponent* opponents, Msg* msg) {
    fprintf(stderr, "Received dowhat\n");
    msg->player = game->pID + TOCHAR;    
    if(msg->type == PURCHASE) {
        game->ownedTokens[0] -= msg->info[PURPLE];
        game->ownedTokens[1] -= msg->info[BROWN];
        game->ownedTokens[2] -= msg->info[YELLOW];
        game->ownedTokens[3] -= msg->info[RED];
        game->wild -= msg->wild;
        remove_card(&game->stack, msg->card);
        free(msg->info);
    } else if(msg->type == TAKE) {
        update_tokens(game, msg->info, opponents, msg->player);
        free(msg->info);
    } else {
        game->wild++;
        opponents[game->pID].wild++;
    }

    char* encodedMsg = encode_player(msg);
    fprintf(stdout, "%s\n", encodedMsg);
    free(msg);
    free(encodedMsg);
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
 * returns: ERR if removecard fails,
 *          OK otherwise
 */
Error bought_card(Game* game, Opponent* opponents, Msg* msg) {
    opponents[(int)(msg->player - TOCHAR)].points += msg->info[POINTS];
    // TODO update tokens

    Error err = OK;

    err = update_tokens(game, msg->info, opponents, msg->player);
    if(err == ERR) {
        return ERR;
    }

    err = remove_card(&game->stack, msg->card);
    if(err == ERR) {
        return ERR;
    }

    return OK;
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
#ifdef TEST
    printf("player has tokens:%d,%d,%d,%d,%d\n",
            tokens[0], tokens[1], tokens[2], tokens[3], wild);
#endif

    int ownedTokens[TOKEN_SIZE];
    memcpy(ownedTokens, tokens, TOKEN_SIZE * sizeof(int));
    int usedWild = 0;
    for(int i = 0; i < TOKEN_SIZE; i++) {
        if(card[i + 2] > ownedTokens[i] + wild - usedWild) {
            return 0;
        }

        if(card[i + 2] > ownedTokens[i]) {
            usedWild += card[i + 2] - ownedTokens[i];
            ownedTokens[i] = 0;
        } else {
            ownedTokens[i] -= card[i + 2];
        }
    }
    
    return 1;
}

/*
 * prints the status of every players and the cards in play
 * params:  game - struct containing relevang game information
 *          opponents - array of structs containing player information
 *          int msgType - type of message preceeding this function call
 */
void print_status(Game* game, Opponent* opponents, int msgType) {
    if(msgType == DOWHAT || msgType == EOG) {
        return;
    }

    print_deck(game->stack.deck, game->stack.numCards);
    for(int i = 0; i < game->pCount; i++) {
        fprintf(stderr, "Player %c:%d:Discounts=%d,%d,%d,%d"
                ":Tokens=%d,%d,%d,%d,%d\n", 
                (char)(opponents[i].id + TOCHAR), opponents[i].points,
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
    int signalList[] = {SIGPIPE};
    init_signal_handler(signalList, 1);
    Error err = OK;
    char* line;
    Msg msg;
    msg.info = (Card)malloc(sizeof(int) * CARD_SIZE);
    Opponent* opponents = init_opponents(game->pCount);
    while(err == OK) {
        line = read_line(stdin);
        if(line == NULL) {
            err = E_COMMERR;
            break;
        }
        if((int)decode_hub_msg(&msg, line) == ERR) {
            err = E_COMMERR;
            break;
        }
        switch(msg.type) {
            case EOG:
                err = print_winners(game->pCount, opponents);
                break;
            case DOWHAT:
                err = send_move(game, opponents, playerMove(game));
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
                err = update_tokens(game, msg.info, opponents, msg.player);
                break;
            case WILD:
                update_wild(opponents, msg.player);
                break;
            default:
                err = E_COMMERR;
        }
        print_status(game, opponents, msg.type);
    }
    free(msg.info);
    free(opponents);
    return err;
}
