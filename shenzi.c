#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "err.h"
#include "common.h"
#include "playerCommon.h"
#include "comms.h"
#include "card.h"

// gameplay:
// start loop
//      1. newcard (show all faceup cards)
//      start loop
//          2. ask player for action
//          3. inform all OTHER players of action
//      end loop
// end loop

/*
 * TODO shenzi_move determines the next move to take for shenzi
 * params:  game - struct containing game relevant information
 * returns: msg containing move for this player
 */
Msg* shenzi_move(Game* game) {
    //1. buy card with most points (if enough tokens)
    //  a. smallest cost
    //  b. most recent
    //2. if 3 tokens can be taken
    //  a. purple
    //  b. brown
    //  c. yellow
    //  d. red
    //3. take wild
    printf("shenzi[%d] move\n", game->pID);
    Msg* msg = (Msg*)malloc(sizeof(Msg));
    int biggestCard = -1;
    int lowestCost = -1;
    for(int i = 0; i < game->stack.numCards; i++) {
        // TODO look for valid cards
        // msg->type = PURCHASE;
    }

    int tokenOrder[] = {PURPLE - 2, BROWN - 2, YELLOW - 2, RED - 2};
    int* tokens = get_tokens(game->tokens, tokenOrder);
    if(tokens) {
        msg->type = TAKE;
        msg->info = (Card)malloc(sizeof(int) * CARD_SIZE);
        for(int i = PURPLE; i < CARD_SIZE; i++) {
            msg->info[i] = tokens[i - 2];
        }
        free(tokens);
    } else {
        msg->type = WILD;
    }
    return msg;
}

int main(int argc, char** argv) {
    if(argc != 3) {
        perr_msg(E_ARGC, SHENZI);
        return E_ARGC;
    }
    
    int pCount = check_pcount(argv[1]);
    if (pCount == ERR) {
        perr_msg(E_PCOUNT, SHENZI);
        return E_PCOUNT;
    }

    int pID = check_pid(argv[2], pCount);
    if(pID == ERR) {
        perr_msg(E_PID, SHENZI);
        return E_PID;
    }

    Game game;
    init_player_game(pID, pCount, &game);

    Error err = play_game(&game, &shenzi_move);
    if(err == UTIL) {
        err = OK;
    }
    
    if(game.stack.numCards) {
        shred_deck(game.stack.deck, game.stack.numCards);
    }
    perr_msg(err, SHENZI); 
    return err;
}
