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
 * removes cards player cannot afford
 * params:  stack - struct containing deck and number of cards
 *          sortedCards - cards sorted by point value
 *          validCardNum - pointer to store affordable cards number into
 *          ownedTokens - players owned tokens
 *          wild - players owned wild tokens
 * returns: an array of cards of validCardNum length the player can afford 
 */
int* remove_invalid(Stack* stack, int* sortedCards, int* validCardNum, 
        int* discount, int* ownedTokens, int wild) {
    validCardNum[0] = 0;
    int* validCards = (int*)malloc(sizeof(int) * stack->numCards);
    memset(validCards, -1, sizeof(int) * stack->numCards);
    for(int i = 0; i < stack->numCards; i++) {
        if(can_afford(stack->deck[sortedCards[i]], discount, 
                ownedTokens, wild) > -1) {
            validCards[validCardNum[0]] = sortedCards[i];
            validCardNum[0]++;
        }
    }
    
    return validCards;
}

/*
 * checks if cards can be purchased and chooses one if so
 * params:  game - struct containing game relevant information
 * returns: -1 if no cards are valid,
 *          otherwise, returns a number from 0-7 (unless hub is naughty?)
 */
int choose_card(Game* game) {
    int* sortedCards = sort_by_points(game->stack.numCards, game->stack.deck);
    int validCardNum = 0;
    int duplicates = 0;
    int* validCards = remove_invalid(&game->stack, sortedCards, &validCardNum,
            game->discount, game->ownedTokens, game->wild);
    int chosenCard = -1;
    if(validCardNum == 1) {
        chosenCard = validCards[0];
    } else if(validCardNum > 1) { // check for duplicates
        int max = game->stack.deck[validCards[0]][POINTS];
        for(int i = 0; i < validCardNum; i++) {
            if(game->stack.deck[validCards[i]][POINTS] == max) {
                duplicates++;
            }
        }

        int min = sum_tokens(game->stack.deck[validCards[0]], game->discount);
        chosenCard = validCards[0];
        for(int i = 0; i < duplicates; i++) { // find cheapest
            int sum = sum_tokens(game->stack.deck[validCards[i]], 
                    game->discount);
            if(min >= sum) {
                min = sum;
                chosenCard = validCards[i];
            }
        }

    }

#ifdef TEST
    for(int i = 0; i < validCardNum && validCardNum; i++) {
        fprintf(stderr, "cost:%d, ", 
                sum_tokens(game->stack.deck[validCards[i]], game->discount));
        print_card(game->stack.deck[validCards[i]], validCards[i]);
    }
    fprintf(stderr, "can afford %d cards, had %d duplicates, chose %d\n", 
            validCardNum, duplicates, chosenCard);
#endif
    
    free(validCards);
    free(sortedCards);
    return chosenCard;
}

/*
 * shenzi_move determines the next move to take for shenzi
 * params:  game - struct containing game relevant information
 * returns: msg containing move for this player
 */
Msg* shenzi_move(Game* game, ...) {
#ifdef TEST
    fprintf(stderr, 
            "shenzi[%d] move, tokens:%d,%d,%d,%d,%d, discount:%d,%d,%d,%d, "
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

    Msg* msg = (Msg*)malloc(sizeof(Msg));
    int chosenCard = choose_card(game);
    if(chosenCard > -1) { // if valid card found
        msg->type = PURCHASE;
        msg->card = chosenCard;
        msg->info = (Card)malloc(sizeof(int) * CARD_SIZE);
        msg->wild = can_afford(game->stack.deck[chosenCard], game->discount,
                game->ownedTokens, game->wild);
        int* usedTokens = get_card_cost(game->discount, game->ownedTokens,
                game->stack.deck[chosenCard]);
        memcpy(msg->info + 2, usedTokens, sizeof(int) * TOKEN_SIZE);
        free(usedTokens); 
    } else { // take tokens
        int tokenOrder[] = {PURPLE - 2, BROWN - 2, YELLOW - 2, RED - 2};
        int* tokens = get_tokens(game->tokens, tokenOrder);
        if(tokens) {
            msg->type = TAKE;
            msg->info = (Card)malloc(sizeof(int) * CARD_SIZE);
            memcpy(msg->info + 2, tokens, sizeof(int) * TOKEN_SIZE);

#ifdef TEST
            fprintf(stderr, "taking:\t%d,%d,%d,%d\n", 
                    tokens[0], tokens[1], tokens[2], tokens[3]);
#endif

            free(tokens);
        } else { // take wild token
            msg->type = WILD;
        }
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
    } else {
        free(game.stack.deck);
    }

#ifdef TEST
    fprintf(stderr, "shenzi exiting\n");
#endif

    perr_msg(err, SHENZI); 
    return err;
}
