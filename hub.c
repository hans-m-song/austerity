#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include "err.h"
#include "card.h"
#include "common.h"
#include "comms.h"
#include "hub.h"
#include "signalHandler.h"

/*
 * checks invocation arguments and saves into session memory
 * params:  argv - array of invocation arguments
 *          game - struct containing game relevant information
 *          session - struct containing session only information
 * returns: E_ARGV if any invalid arguments, 
 *          E_DECKIO if deck cannot be accessed, 
 *          E_DECKR if invalid deck contents,
 *          E_EXEC if players couldn't be started
 *          OK otherwise
 */
Error hub_init(char** argv, Game* game) {
    char* temp;
    game->pCount = 0;
    long int numTokens = strtol(argv[1], &temp, 10);
    long int numPoints = strtol(argv[2], &temp, 10);

    if(numTokens < 0 || numTokens > INT_MAX || !is_all_num(argv[1]) ||
            numPoints < 0 || numPoints > INT_MAX || !is_all_num(argv[2])) {
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
    game->stack.deck = (Deck)malloc(sizeof(Card));
    game->hubStack.numCards = 0;
    game->hubStack.deck = (Deck)malloc(sizeof(Card));
    if(read_deck(deckFile, &game->hubStack) != OK) {
        fclose(deckFile);
        return E_DECKR;
    }
    fclose(deckFile);

#ifdef VERBOSE
    print_deck(game->stack.deck, game->stack.numCards);
#endif
        
    return OK;
}

/*
 * sends the message to all players in order
 * params:  pCount - number of players to send message to
 *          players - array of players
 *          msg - struct containing message contents
 * returns: E_DEADPLAYER if player closed pipe unexpectedly
 *          OK otherwise
 */
Error broadcast(int pCount, Player* players, Msg* msg) {
    Error err;
    for(int i = 0; i < pCount; i++) {
        err = send_msg(msg, players[i].toChild);
    }

    if(err) {
        return E_DEADPLAYER;
    }

    return OK;
}

/*
 * TODO send_msg takes the message and pipes it through the given fd
 * params:  msg - message to send
 *          destination - pipe to write to
 * returns: E_DEADPLAYER if pipe closed unexpectedly
 *          OK otherwise
 */
Error send_msg(Msg* msg, FILE* destination) {
    char* encodedMsg = encode_hub(msg);
    if(!encodedMsg) {
        return E_PROTOCOL;
    }

#ifdef TEST
    printf("sending to child: %s\n", encodedMsg);
#endif

    fprintf(destination, "%s\n", encodedMsg);
    fflush(destination);
    free(encodedMsg);
    
    int signal = check_signal();
    if(signal) {
        return signal;
    }

    return OK;
}
