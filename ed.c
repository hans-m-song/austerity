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
 * checks if player can ALMOST afford the card with their owned tokens
 * mainly for ed if the player cant find a card it can buy outright
 * params:  card - card to purchase
 *          discount - discount from owned cards
 *          tokens - players owned tokens
 *          tokenOrder - order in which to check for tokens
 *          wild - number of owned wild tokens
 * returns: NULL if cannot ALMOST afford,
 *          int array of tokens used otherwise
 */
int* can_almost_afford(Card card, int* discount, int* tokens, 
        int* tokenOrder, int wild) {
    int usedWild = 0;
    int totalDebt = 0; // total number of tokens owed
    int* debt = (int*)calloc(TOKEN_SIZE, sizeof(int)); // which tokens owed
    for(int i = 0; i < TOKEN_SIZE; i++) {
        int cost = card[tokenOrder[i] + 2] - discount[tokenOrder[i]];
        int sum = cost - tokens[tokenOrder[i]] + wild - usedWild; // balance 
        if(sum > 1) { // greater than a single draw
            free(debt);
            return NULL;
        }

        if(sum == 1) { // can be covered from a single draw
            totalDebt++;
            if(totalDebt > 3) { // debt already exceeded
                free(debt);
                return NULL;
            }

            if(cost > tokens[tokenOrder[i]]) { // wild required
                usedWild += cost - tokens[tokenOrder[i]];
            }
            debt[tokenOrder[i]] = 1;
        }
    }

    if(totalDebt < 3) {
        for(int i = 0; i < TOKEN_SIZE; i++) {
            if(totalDebt < 3 && debt[tokenOrder[i]] != 1) {
                debt[tokenOrder[i]] = 1;
                totalDebt++;
            }
        }
    }

    return debt;
}

/*
 * removes cards all players cannot afford, 
 * sorted by player order starting from this player, finds card with highest
 * points affordable by each player
 * params:  game - struct containing game relevant information
 *          opponents - record of all players stats
 *          sortedCards - cards sorted by point value
 *          validCardNum - pointer to store affordable cards number into
 *          ownedTokens - players owned tokens
 *          wild - players owned wild tokens
 * returns: an array of cards of validCardNum length each player can afford
 *          sorted by points (dependent on player)
 */
int* remove_invalid(Game* game, Opponent* opponents, 
        int* sortedCards, int* validCardNum) {
    int* validCards = (int*)calloc(game->stack.numCards, sizeof(int));
    for(int i = 0; i < game->pCount; i++) { // for every player
        int nextPlayer = (i + game->pID + 1) % game->pCount;
        for(int j = 0; j < game->stack.numCards; j++) { // find highest points 
            if(!has_element(validCards, validCardNum[0], sortedCards[j])) {
                if(can_afford(game->stack.deck[sortedCards[j]], 
                        opponents[nextPlayer].discount, 
                        opponents[nextPlayer].tokens, 
                        opponents[nextPlayer].wild) > -1) {
                    validCards[validCardNum[0]] = sortedCards[j];
                    validCardNum[0]++;
                    break;
                }
            }
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
int choose_card(Game* game, Opponent* opponents) {
    int* sortedCards = sort_by_points(game->stack.numCards, game->stack.deck);
    int validCardNum = 0;
    int* validCards = remove_invalid(game, opponents, 
            sortedCards, &validCardNum);
    int chosenCard = -100; // out of bounds, no chance of valid card
    for(int i = 0; i < validCardNum; i++) { // find card this player can buy
        if(can_afford(game->stack.deck[validCards[i]], game->discount, 
                game->tokens, game->wild) > -1) {
            chosenCard = validCards[i];
            break;
        }
    }
    
    free(sortedCards);
    free(validCards);
    return chosenCard;
}

/*
 * returns tokens ed should get if no cards can be bought outright
 * params:  game - struct containing game relevant information
 *          opponents - record of all players stats
 *          tokenOrder - order in which to check for tokens
 * returns: int array of tokens to take
 */
int* choose_alternate_card(Game* game, Opponent* opponents, int* tokenOrder) {
    int* debt;
    int* sortedCards = sort_by_points(game->stack.numCards, game->stack.deck);
    int validCardNum = 0;
    int* validCards = remove_invalid(game, opponents, 
            sortedCards, &validCardNum);
    for(int i = 0; i < game->pCount; i++) { // for each player find
        for(int j = 0; j < validCardNum; j++) { // find almost buyable
            debt = can_almost_afford(game->stack.deck[validCards[i]], 
                    game->discount, game->tokens, tokenOrder, game->wild);
            if(debt) { // alternate found 
                free(sortedCards);
                free(validCards);
                return debt;
            }
        }
    }

    free(sortedCards);
    free(validCards);
    return NULL;
}

/*
 * TODO ed_move determines the next move to take for ed
 * params:  game - struct containing game relevant information
 * returns: msg containing move for this player
 */
Msg* ed_move(Game* game, ...) {
    va_list args;
    va_start(args, game);
    Opponent* opponents = va_arg(args, Opponent * );
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

    Msg* msg = (Msg*)malloc(sizeof(Msg));
    int chosenCard = choose_card(game, opponents);
    if(chosenCard > -1) { // valid card found
        msg->type = PURCHASE;
        msg->card = chosenCard;
        msg->info = (Card)malloc(sizeof(int) * CARD_SIZE);
        msg->wild = can_afford(game->stack.deck[chosenCard], game->discount,
                game->ownedTokens, game->wild);
        int* usedTokens = get_card_cost(game->discount, game->ownedTokens,
                game->stack.deck[chosenCard]);
        memcpy(msg->info + 2, usedTokens, sizeof(int) * TOKEN_SIZE);
        free(usedTokens);
    } else { // no cards are affordable outright by this player
        int tokenOrder[] = {YELLOW - 2, RED - 2, BROWN - 2, PURPLE - 2};
        int* debt = choose_alternate_card(game, opponents, tokenOrder);
        int* tokens = get_tokens(game->tokens, tokenOrder);
        if(debt) {
            msg->type = TAKE;
            msg->info = (Card)malloc(sizeof(int) * CARD_SIZE);
            memcpy(msg->info + 2, debt, sizeof(int) * TOKEN_SIZE);
            free(debt);
            free(tokens);
        } else if(tokens) {
            msg->type = TAKE;
            msg->info = (Card)malloc(sizeof(int) * CARD_SIZE);
            memcpy(msg->info + 2, tokens, sizeof(int) * TOKEN_SIZE);
            free(tokens);
        } else {
            msg->type = WILD;
        }
    }

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
