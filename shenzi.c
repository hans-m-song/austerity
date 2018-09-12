#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "err.h"
#include "common.h"
#include "playerCommon.h"

/*
 * main logic for gameplay of shenzi
 * params:  pCount - number of players
 *          pID - ID of this player
 * returns: E_PIPECLOSE if pipe closed unexpectedly,
 *          OK otherwise for end of game
 */
Error play_game() { //Game* game) {


    return OK;
}

int main(int argc, char** argv) {
    if(argc != 3) {
        perr_msg(E_ARGC, "shenzi");
        return E_ARGC;
    }
    
    int pCount = check_pcount(argv[1]);
    if (pCount == ERR) {
        perr_msg(E_PCOUNT, "shenzi");
        return E_PCOUNT;
    }

    int pID = check_pid(argv[2], pCount);
    if(pID == ERR) {
        perr_msg(E_PID, "shenzi");
        return E_PID;
    }

#ifdef TEST
    printf("shenzi[%d] launched successfully\n", pID);
#endif

    Error err = play_game();
    
    perr_msg(err, "shenzi"); 
    return err;
}
