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
 * sorts deck by lowest cost, newest first
 * params:  numCards - number of cards in deck
 *          deck - array of cards to sort
 * returns: array of indicies of the cards in sorted order
 */
int* sort_by_cost(int numCards, Deck deck) {
    int* sortedCards = (int*)malloc(numCards * sizeof(int));
    memset(sortedCards, -1, sizeof(int) * numCards);
    for(int i = numCards - 1; i > -1; i--) {
        int max = 0; 
        for(int j = 0; j < numCards; j++) {
            if(!has_element(sortedCards, numCards, j)) {
                int sum = sum_tokens(deck[j]); 
                if(max < sum) {
                    max = sum;
                    sortedCards[i] = j;
                }
            }
        }
    }

#ifdef TEST 
    printf("sorted deck:\n");
    for(int i = 0; i < numCards; i++) {
        fprintf(stderr, "cost:\t%d = ", 
                sum_tokens(deck[sortedCards[i]]));
        print_card(deck[sortedCards[i]], sortedCards[i]);
    }
    printf("\n");
#endif

    return sortedCards;
}

/*
 * checks if cards can be purchased and chooses one if so
 * params:  game - struct containing game relevant information
 * returns: -1 if no cards are valid,
 *          otherwise, returns a number from 0-7 (unless hub is naughty?)
 */
int choose_card(Game* game) {
    int* sortedCards = sort_by_cost(game->stack.numCards, game->stack.deck);

    int* validCards = (int*)malloc(sizeof(int) * game->stack.numCards);
    memset(validCards, -1, sizeof(int) * game->stack.numCards);
    int minCost = -1;
    int validCardNum = 0; // prune unaffordable cards
    for(int i = 0; i < game->stack.numCards; i++) {
        if(can_afford(game->stack.deck[sortedCards[i]],
                    game->ownedTokens, game->wild)) {
            validCards[validCardNum++] = sortedCards[i];
        }
    }

    int chosenCard = -1;
    if(minCost > 0) {
        int max = 0;
        for(int i = 0; i < validCardNum; i++) {
            print_card(game->stack.deck[validCards[i]], validCards[i]);
            if(max < game->stack.deck[validCards[i]][POINTS]) {
                max = game->stack.deck[validCards[i]][POINTS];
                chosenCard = validCards[i];
            }
        }
    }

#ifdef TEST
    printf("can afford %d cards, chose %d\n", validCardNum, chosenCard);
#endif
    
    free(validCards);
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

    int signalList[] = {SIGPIPE};
    init_signal_handler(signalList, 1);
    
    Error err = play_game(&game, &shenzi_move);
    if(err == UTIL) {
        err = OK;
    }

    if(err == ERR) {
        err = E_COMMERR;
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
