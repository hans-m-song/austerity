#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "err.h"
#include "card.h"
#include "common.h"

/*
 * checks invocation arguments and saves into session memory
 * params:  argc - number of invocation arguments
 *          argv - array of invocation arguments
 *          game - struct containing game relevant information
 * returns: E_ARGV if any invalid arguments, 
 *          E_DECKIO if deck cannot be accessed, 
 *          E_DECKR if invalid deck contents,
 *          E_EXEC if players couldn't be started
 *          OK otherwise
 */
Error hub_init(int argc, char** argv, Game* game) {
    char* temp;
    game->pCount = argc - 4;
    long int numTokens = strtol(argv[1], &temp, 10);
    long int numPoints = strtol(argv[2], &temp, 10);

    if(numTokens < 0 || numTokens > INT_MAX ||
            numPoints < 0 || numPoints > INT_MAX) {
        return E_ARGV;
    }
    for(int i = 0; i < TOKEN_SIZE; i++) {
        game->tokens[i] = (int)numTokens;
    }
    game->numPoints = (int)numPoints;

    FILE* deckFile = fopen(argv[3], "r");
    if(!deckFile) {
        return E_DECKIO;
    }

    game->stack.numCards = 0;
    if(read_deck(deckFile, &game->stack) != OK) {
        shred_deck(game->stack.deck, game->stack.numCards);
        return E_DECKR;
    }
    fclose(deckFile);

#ifdef TEST
    print_deck(game->stack.deck, game->stack.numCards);
#endif

    memset(game->tokens, 0, sizeof(int) * TOKEN_SIZE);
        
    return OK;
}
