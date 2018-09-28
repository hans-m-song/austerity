#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <stdarg.h>
#include "err.h"
#include "common.h"
#include "playerCommon.h"
#include "comms.h"
#include "card.h"
#include "token.h"
#include "signalHandler.h"

/*
 * removes cards all players cannot afford
 * params:  game - struct containing game relevant information
 *          opponents - record of all players stats
 *          sortedCards - cards sorted by point value
 *          validCardNum - pointer to store affordable cards number into
 *          ownedTokens - players owned tokens
 *          wild - players owned wild tokens
 * returns: an array of cards of validCardNum length the player can afford 
 */
int* remove_invalid(Game* game, Opponent* opponents, 
        int* sortedCards, int* validCardNum) {
    int* validCards = (int*)calloc(game->stack.numCards, sizeof(int));
    for(int i = 0; i < game->stack.numCards; i++) {
        int canAffordCount = 0;
        for(int j = 0; j < game->pCount; j++) {
            if(can_afford(game->stack.deck[sortedCards[i]], 
                    opponents[j].tokens, opponents[j].discount, 
                    opponents[j].wild) > -1) {
                canAffordCount++;
            }
        }
        if(canAffordCount) {
            validCards[validCardNum[0]] = sortedCards[i];
            validCardNum[0]++;
        }
    }

    return validCards;
}

/*
 * checks if cards can be purchased and chooses one if so
 * params:  game - struct containing game relevant information
 *          opponents - record of all players stats
 * returns: -1 if no cards are valid,
 *          otherwise, returns a number from 0-7 (unless hub is naughty?)
 */
int choose_card() {//Game* game, Opponent* opponents) {
    /*
    int* sortedCards = sort_by_points(game->stack.numCards, game->stack.deck);
    int validCardNum = 0;
    int duplicates = 0;
    int* validCards = remove_invalid(game, opponents, 
            sortedCards, &validCardNum);
    */
    return -1;
}

/*
 * TODO ed_move determines the next move to take for ed
 * params:  game - struct containing game relevant information
 * returns: msg containing move for this player
 */
Msg* ed_move(Game* game, ...) {
    va_list args;
    va_start(args, game);
    //Opponent* opponents = va_arg(args, Opponent*);
    va_end(args);

#ifdef TEST
    fprintf(stderr, 
            "ed[%d] move, tokens:%d,%d,%d,%d,%d, discount:%d,%d,%d,%d, "
            "bank:%d,%d,%d,%d\n", 
            game->pID,
            game->ownedTokens[0], game->ownedTokens[1], 
            game->ownedTokens[2], game->ownedTokens[3],
            game->wild,
            game->discount[0], game->discount[1],
            game->discount[2], game->discount[3],
            game->tokens[0], game->tokens[1], 
            game->tokens[2], game->tokens[3]);
#endif

    //int chosenCard = choose_card(game, opponents);

    Msg* msg = (Msg*)malloc(sizeof(Msg));
    msg->type = WILD;
    msg->player = game->pID + TOCHAR;
    return msg;
}

int main(int argc, char** argv) {
    if(argc != 3) {
        perr_msg(E_ARGC, ED);
        return E_ARGC;
    }
    
    int pCount = check_pcount(argv[1]);
    if (pCount == ERR) {
        perr_msg(E_PCOUNT, ED);
        return E_PCOUNT;
    }

    int pID = check_pid(argv[2], pCount);
    if(pID == ERR) {
        perr_msg(E_PID, ED);
        return E_PID;
    }

    Game game;
    init_player_game(pID, pCount, &game);

    int signalList[] = {SIGPIPE};
    init_signal_handler(signalList, 1);
    
    Error err = play_game(&game, &ed_move);
    if(err == UTIL) {
        err = OK;
    }

    if(err == ERR) {
        err = E_COMMERR;
    }
    
    if(game.stack.numCards) {
        shred_deck(game.stack.deck, game.stack.numCards);
    } else {
        free(game.stack.deck);
    }

#ifdef TEST
    fprintf(stderr, "ed exiting\n");
#endif

    perr_msg(err, ED); 
    return err;
}
