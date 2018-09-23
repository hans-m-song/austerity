#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "err.h"
#include "common.h"
#include "playerCommon.h"
#include "comms.h"
#include "card.h"
#include "token.h"

/*
 * checks if cards can be purchased and chooses one if so
 * params:  game - struct containing game relevant information
 * returns: -1 if no cards are valid,
 *          otherwise, returns a number from 0-7 (unless hub is naughty?)
 */
int choose_card(Game* game) {
    int* sortedCards = (int*)calloc(game->stack.numCards, sizeof(int));
    for(int i = 0; i < game->stack.numCards; i++) {
        int max = 0;
        for(int j = 0; j < game->stack.numCards; j++) { // descending cost
            if(has_element(sortedCards, game->stack.numCards, j)) {
                continue; // skip if already in sortedCards
            }
            int sum = sum_tokens(game->stack.deck[j]); 
            if(max < sum) {
                max = sum;
                sortedCards[i] = j;
            }
        }
    }

#ifdef VERBOSE
    printf("sorted deck:\n");
    for(int i = 0; i < game->stack.numCards; i++) {
        print_card(game->stack.deck[sortedCards[i]]);
    }
    printf("\n");
#endif
    
    int validCardNum = game->stack.numCards; // prune unaffordable cards
    for(int i = 0; i < game->stack.numCards; i++) {
        if(!can_afford(game->stack.deck[sortedCards[i]], 
                game->ownedTokens, game->wild)) {
            sortedCards[i] = -1;
            validCardNum--;
        }
    }

    int max = 0;
    int chosenCard = -1;
    for(int i = game->stack.numCards - 1; i > -1; i--) { // reversed to ascend
        if(sortedCards[i] > 0 && // choose biggest from remaining
                game->stack.deck[sortedCards[i]][POINTS] > max) {
            chosenCard = sortedCards[i];
        }
    }

#ifdef TEST
    printf("can afford %d cards, chose %d\n", validCardNum, chosenCard);
#endif
    
    free(sortedCards);
    return chosenCard;
}

/*
 * TODO shenzi_move determines the next move to take for shenzi
 * params:  game - struct containing game relevant information
 * returns: msg containing move for this player
 */
Msg* shenzi_move(Game* game) {
#ifdef TEST
    printf("shenzi[%d] move\n", game->pID);
#endif

    Msg* msg = (Msg*)malloc(sizeof(Msg));
    int chosenCard = choose_card(game);
    if(chosenCard > 0) { // if valid card found
        msg->type = PURCHASE;
        msg->card = chosenCard;
        msg->info = (Card)malloc(sizeof(int) * CARD_SIZE);
        memcpy(msg->info, game->stack.deck[chosenCard], 
                sizeof(int) * CARD_SIZE);
    } else { // take tokens
        // TODO figure out why shenzi doesnt take tokens
        int tokenOrder[] = {PURPLE - 2, BROWN - 2, YELLOW - 2, RED - 2};
        int* tokens = get_tokens(game->tokens, tokenOrder);
        if(tokens) {
            msg->type = TAKE;
            msg->info = (Card)malloc(sizeof(int) * CARD_SIZE);
            /*for(int i = 0; i < TOKEN_SIZE; i++) {
                msg->info[i + 2] = tokens[i];
            }*/
            memcpy(msg->info + 2, tokens, sizeof(int) * TOKEN_SIZE);
#ifdef TEST
            printf("taking:\t%d,%d,%d,%d\n", 
                    tokens[0], tokens[1], tokens[2], tokens[3]);
#endif
            free(tokens);
        } else { // take wild token
            msg->type = WILD;
        }
    }

#ifdef VERBOSE 
    printf("doing move %d\n", msg->type);
#endif

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

#ifdef TEST
    printf("shenzi exiting\n");
#endif

    perr_msg(err, SHENZI); 
    return err;
}
