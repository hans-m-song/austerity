#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <limits.h>
#include <signal.h>
#include <unistd.h>
#include "err.h"
#include "common.h"
#include "card.h"
#include "comms.h"
#include "hub.h"
#include "signalHandler.h"

/*
 * clears memory used by hub
 * params:  game - struct containing relevant game information
 *          session - struct containing hub only information
 *          err - code to exit with
 */
void end_game(Game* game, Session* session, Error err) {
    shred_deck(game->stack.deck, game->stack.numCards);

    if(session->parentPID == getpid()) {
        kill_players(game->pCount, session->players, err);
        herr_msg(err);
    }
    
    free(session->players);
    
    if(err) {
        exit(err);
    }

#ifdef TEST
    fprintf(stderr, "[%d]exit:\tgot code %d\n", getpid(), err);
#endif
}

/*
 * creates the message to send available tokens to players
 * params:  game - struct containing relevant game information
 *          session - struct containing hub only information
 *          tokens - number of tokens available
 * returns: E_DEADPLAYER if client disconnects,
 *          OK otherwise
 */
Error send_tokens(int pCount, Player* players, int tokens) {
    Msg msg;
    msg.type = TOKENS;
    msg.tokens = tokens;
    return broadcast(pCount, players, &msg); 
}

/*
 * creates the message to send a card to players
 * params:  game - struct containing relevant game information
 *          session - struct containing hub only information
 *          card - card in stack to send 
 *          (if -1, send 8 cards, if saved is less than 8, send all)
 * returns: E_DEADPLAYER if client disconnects,
 *          OK otherwise
 */
Error send_card(Game* game, Session* session, int card) {
    Error err = OK;
    Msg msg;
    msg.type = NEWCARD;
    msg.info = (Card)malloc(sizeof(int) * CARD_SIZE);
    if(card > -1) {
        memcpy(msg.info, game->stack.deck[card], sizeof(int) * CARD_SIZE);
        err = broadcast(game->pCount, session->players, &msg);
    } else {
        for(int i = 0; i < game->stack.numCards && i < 8; i++) {
            memcpy(msg.info, game->stack.deck[i], sizeof(int) * CARD_SIZE);
            err = broadcast(game->pCount, session->players, &msg);
            if(err != OK) {
                break;
            }
        }
    }
    
    free(msg.info);
    return err;
}

/*
 * main logic for the hub
 * params:  game - struct containing relevant game information
 *          session - struct containing hub only information
 * returns: E_DEADPLAYER if client disconnects,
 *          E_PROTOCOL if client is being naughty
 *          E_SIGINT if sigint was caught
 *          OK otherwise for end of stack
 */
Error start_hub(Game* game, Session* session) {
    int signal = check_signal();
    if(signal) {
        return signal;
    }

    if(send_tokens(game->pCount, session->players, game->tokens[0]) ||
            send_card(game, session, -1)) { // pre-game setup
        return E_DEADPLAYER;
    }

    Error err = OK;
    while((signal = check_signal()), !signal && err == OK) {
        // TODO game loop
        // do loop here probably
        return OK;
    }
    
    if(!err) {
        return signal;
    }

    return err;
}

int main(int argc, char** argv) {
    Game game;
    Session session;
    session.parentPID = getpid();

#ifdef TEST
    printf("pPID:\t%d\n", getpid());
#endif
    
    if(argc < 6 || argc - 4 > MAX_PLAYERS) {
        herr_msg(E_ARGC);
        return E_ARGC;
    }
    
    Error err = OK;
    err = hub_init(argv, &game);
    if(err) {
        herr_msg(err);
        return err;
    }
    
    int signalList[] = {SIGINT, SIGPIPE, SIGCHLD};
    init_signal_handler(signalList, 3);

    if(start_players(argc - 4, argv + 4, &game, &session) != OK ||
            getpid() != session.parentPID) {
        end_game(&game, &session, E_EXEC);
    } // no child should go past here

    err = start_hub(&game, &session);
    end_game(&game, &session, err);
    
#ifdef TEST
    printf("eog reached\n");
#endif

    return err;
}
