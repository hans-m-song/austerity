#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "err.h"
#include "common.h"
#include "playerCommon.h"
#include "comms.h"
#include "card.h"

/*
 * TODO ed_move determines the next move to take for ed 
 * params:  game - struct containing game relevant information
 * returns: msg containing move for this player
 */
Msg* ed_move(Game* game) {
    printf("ed[%d] move\n", game->pID);
    Msg* msg = (Msg*)malloc(sizeof(Msg));
    msg->type = DOWHAT;
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

    Error err = play_game(&game, &ed_move);
    if(err == UTIL) {
        err = OK;
    }
    
    if(game.stack.numCards) {
        shred_deck(game.stack.deck, game.stack.numCards);
    }

#ifdef TEST
    printf("ed exiting\n");
#endif

    perr_msg(err, ED); 
    return err;
}
