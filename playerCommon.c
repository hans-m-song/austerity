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
}

/*
 * TODO encodes a message and sends it to the given destination
 * params:  msg - message to encode
 *          destination - where to send the message to
 */
void send_move(Msg* msg) {
    char* encodedMsg = encode_player(msg);
    // send msg
#ifdef TEST
    printf("send:\t%s\n", encodedMsg);
#endif
}

/*
 * main driver for logic of players
 * params:  game - struct containing game relevant information
 *          move - function pointer to the player-specific move logic
 * returns: E_COMMERR if bad message received,
 *          OK otherwise for end of game
 */
Error play_game(Game* game, Msg* (*playerMove)(Game*)) {
    Error err = E_COMMERR;
    char* line;
    Msg msg; 
    while(1) {
        line = read_line(stdin);
        if(!line) {
            break;
        }
            
        if((int)decode_hub_msg(&msg, line) == ERR) {
            break;
        }
        
        switch(msg.type) {
            case EOG:
                return OK;
            case DOWHAT:
                send_move(playerMove(game));
                break;
            case TOKENS:
                break;
            case NEWCARD:
                add_card(&game->stack, msg.info[COLOR], msg.info[POINTS], 
                        msg.info[PURPLE], msg.info[BROWN], 
                        msg.info[YELLOW], msg.info[RED]);
                free(msg.info);
                break;
            case PURCHASED:
                // TODO remove_card();
                free(msg.info);
                break;
            case TOOK:
                // TODO update_tokens();
                free(msg.info);
                break;
            case WILD:
                // TODO update_tokens();
                break;
            default:
                return E_COMMERR;
        }
    }
    return err;
}
