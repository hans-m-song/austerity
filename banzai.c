#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "err.h"
#include "common.h"
#include "playerCommon.h"
#include "comms.h"
#include "card.h"

Msg* banzai_move(Game* game) {
    printf("banzai[%d] move\n", game->pID);
    Msg* msg = (Msg*)malloc(sizeof(Msg));
    msg->type = DOWHAT;
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

#ifdef TEST
    printf("banzai[%d] launched successfully\n", pID);
#endif

    Game game;
    init_player_game(pID, pCount, &game);

    Error err = play_game(&game, &banzai_move);
    
    perr_msg(err, BANZAI); 
    return err;
}
