#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "err.h"
#include "common.h"
#include "playerCommon.h"
#include "comms.h"
#include "card.h"
#include "token.h"
#include "signalHandler.h"

/*
 * TODO banzai_move determines the next move to take for banzai
 * params:  game - struct containing game relevant information
 * returns: msg containing move for this player
 */
Msg* banzai_move(Game* game) {
#ifdef TEST
    printf("banzai[%d] move, tokens:%d,%d,%d,%d,%d\n", game->pID,
            game->ownedTokens[0], game->ownedTokens[1], 
            game->ownedTokens[2], game->ownedTokens[3],
            game->wild);
#endif

    Msg* msg = (Msg*)malloc(sizeof(Msg));
    msg->type = WILD;
    msg->player =game->pID + TOCHAR;
    return msg;
}

int main(int argc, char** argv) {
    if(argc != 3) {
        perr_msg(E_ARGC, BANZAI);
        return E_ARGC;
    }
    
    int pCount = check_pcount(argv[1]);
    if (pCount == ERR) {
        perr_msg(E_PCOUNT, BANZAI);
        return E_PCOUNT;
    }

    int pID = check_pid(argv[2], pCount);
    if(pID == ERR) {
        perr_msg(E_PID, BANZAI);
        return E_PID;
    }

    Game game;
    init_player_game(pID, pCount, &game);

    int signalList[] = {SIGPIPE};
    init_signal_handler(signalList, 1);
    
    Error err = play_game(&game, &banzai_move);
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
    printf("banzai exiting\n");
#endif

    perr_msg(err, BANZAI); 
    return err;
}
