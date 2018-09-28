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
 * removes cards player cannot afford and cards worth 0
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
                ownedTokens, wild) > -1 && 
                stack->deck[sortedCards[i]][POINTS] > 0) {
            validCards[validCardNum[0]] = sortedCards[i];
            validCardNum[0]++;
        }
    }
    
    return validCards;
}

/*
 * sorts deck by cost, highest and oldest first
 * params:  numCards - number of cards in deck
 *          deck - array of cards to sort
 *          discount - players discount on cards
 * returns: array of indicies of the cards in sorted order
 */
int* sort_by_cost(int numCards, Deck deck, int* discount) {
    int* sortedCards = (int*)malloc(numCards * sizeof(int));
    memset(sortedCards, -1, sizeof(int) * numCards);
    for(int i = 0; i < numCards; i++) {
        int max = 0;
        for(int j = numCards - 1; j > -1; j--) {
            if(!has_element(sortedCards, numCards, j)) {
                if(max < sum_tokens(deck[j], discount)) {
                    max = sum_tokens(deck[j], discount);
                    sortedCards[i] = j;
                }
            }
        }
    }

    return sortedCards;
}

/*
 * checks if cards can be purchased and chooses one if so
 * params:  game - struct containing game relevant information
 * returns: -1 if no cards are valid,
 *          otherwise, returns a number from 0-7 (unless hub is naughty?)
 */
int choose_card(Game* game) {
    int* sortedCards = sort_by_cost(game->stack.numCards, 
            game->stack.deck, game->discount);
    int validCardNum = 0;
    int duplicates = 0;
    int* validCards = remove_invalid(&game->stack, sortedCards, 
            &validCardNum, game->discount, game->ownedTokens, game->wild);
    int chosenCard = -1;
    if(validCardNum == 1) {
        chosenCard = validCards[0];
    } else if(validCardNum > 1) { // check for duplicates
        int max = sum_tokens(game->stack.deck[validCards[0]], game->discount);
        for(int i = 0; i < validCardNum; i++) {
            if(sum_tokens(game->stack.deck[validCards[i]], 
                        game->discount) == max) {
                duplicates++;
            }
        }

        int wild = can_afford(game->stack.deck[validCards[0]], game->discount, 
                game->ownedTokens, game->wild);
        chosenCard = validCards[0];
        for(int i = 0; i < duplicates; i++) { // find whichever costs most wild
            int newWild = can_afford(game->stack.deck[validCards[i]], 
                    game->discount, game->ownedTokens, game->wild);
            if(wild <= newWild) {
                wild = newWild;
                chosenCard = validCards[i];
            }

        }
        
    }

    free(validCards);
    free(sortedCards);
    return chosenCard; 
}

/*
 * TODO banzai_move determines the next move to take for banzai
 * params:  game - struct containing game relevant information
 * returns: msg containing move for this player
 */
Msg* banzai_move(Game* game) {
#ifdef TEST
    fprintf(stderr, 
            "banzai[%d] move, tokens:%d,%d,%d,%d,%d, discount:%d,%d,%d,%d, "
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
    int ownedTokenSum = game->ownedTokens[0] + game->ownedTokens[1] + 
        game->ownedTokens[2] + game->ownedTokens[3] + game->wild;
    int tokenOrder[] = {YELLOW - 2, BROWN - 2, PURPLE - 2, RED - 2};
    int* tokens = get_tokens(game->tokens, tokenOrder);
    if(tokens && ownedTokenSum < 3) { // take tokens
        msg->type = TAKE;
        msg->info = (Card)malloc(sizeof(int) * CARD_SIZE);
        memcpy(msg->info + 2, tokens, sizeof(int) * TOKEN_SIZE);

#ifdef TEST
            fprintf(stderr, "taking:\t%d,%d,%d,%d\n", 
                    tokens[0], tokens[1], tokens[2], tokens[3]);
#endif
            free(tokens);
    } else { // take card
        int chosenCard = choose_card(game);
        if(chosenCard > -1) {
            msg->type = PURCHASE;
            msg->card = chosenCard;
            msg->info = (Card)malloc(sizeof(int) * CARD_SIZE);
            msg->wild = can_afford(game->stack.deck[chosenCard], 
                    game->discount, game->ownedTokens, game->wild);
            int* usedTokens = get_card_cost(game->discount, game->ownedTokens,
                    game->stack.deck[chosenCard]);
            memcpy(msg->info + 2, usedTokens, sizeof(int) * TOKEN_SIZE);
            free(usedTokens); 
        } else { // take wild
            msg->type = WILD;
        }
    }
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
    fprintf(stderr, "banzai exiting\n");
#endif

    perr_msg(err, BANZAI); 
    return err;
}
