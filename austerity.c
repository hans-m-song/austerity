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
 *          err - code to exit with
 */
void end_game(Game* game, Session* session, Error err) {
#ifdef TEST
    printf("err:%d\n", err);
#endif

    shred_deck(game->stack.deck, game->stack.numCards);
    shred_deck(game->hubStack.deck, game->hubStack.numCards);

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
    Msg msg = {TOKENS, 0, tokens, 0, 0, 0};
    return broadcast(pCount, players, &msg); 
}

/*
 * creates the message to send a card to players, 
 * moves card from hubstack to stack to keep track of cards in game
 * params:  game - struct containing relevant game information
 *          session - struct containing hub only information
 *          cards - number of cards to send
 * returns: E_DEADPLAYER if client disconnects,
 *          UTIL if utility operations fail
 *          OK otherwise
 */
Error send_card(Game* game, Session* session, int cards) {
    Error err = OK;
    Msg msg = {NEWCARD, 0, 0, 0, 0, 0};
    msg.info = (Card)malloc(sizeof(int) * CARD_SIZE);
    for(int i = 0; i < cards; i++) {
        announce_card(game->hubStack.deck[0]);
        memcpy(msg.info, game->hubStack.deck[0], sizeof(int) * CARD_SIZE);
        if(broadcast(game->pCount, session->players, &msg) != OK ||
                move_card(&game->hubStack, &game->stack, 0) != OK) {
            free(msg.info);
            return UTIL;
        }
    }
    
    free(msg.info);
    return err;
}

/*
 * checks if the players requested move is legal
 * params:  game - struct containing relevant game information
 *          msg - contents of message to check for validity
 * returns: ERR if illegal,
 *          OK otherwise
 */
Error valid_move() { //Game* game, Msg* msg) {
    return OK;
}

/*
 * asks player for their next move, reprompt on invalid move
 * params:  game - struct containing relevant game information
 *          player - player to retrieve response from
 *          response - struct to save message contents to
 * returns: type of message received
 *          ERR if error encountered or reprompt returns invalid move
 */
Comm get_player_move(Game* game, Player player, Msg* response) {
    Msg request = {DOWHAT, 0, 0, 0, 0, 0};
    response->info = (Card)malloc(sizeof(int) * CARD_SIZE);
    send_msg(&request, player.pipeOut[WRITE]);
    
    FILE* source = fdopen(player.pipeIn[READ], "r");
    char* line = read_line(source);
    if(line == NULL) {
        fclose(source);
        return ERR;
    }
        
    if((int)decode_player_msg(response, line) != ERR) {
        if(valid_move(game, response) == OK) {
#ifdef TEST
            printf("got response: %d\n", response->type);
#endif
            
            fclose(source);
            free(line);
            return response->type;
        }
    }
    
    send_msg(&request, player.pipeOut[WRITE]); // reprompt
    if((int)decode_player_msg(response, line) != ERR) {
        if(valid_move(game, response) == OK) {
#ifdef TEST
            printf("got response after reprompt: %d\n", response->type);
#endif
            
            fclose(source);
            free(line);
            return response->type;
        }
    }

    fclose(source);
    free(line);
    return ERR;
}

/*
 * main logic for the hub
 * params:  game - struct containing relevant game information
 *          session - struct containing hub only information
 * returns: E_DEADPLAYER if client disconnects,
 *          E_PROTOCOL if client is being naughty
 *          E_SIGINT if sigint was caught
 *          UTIL otherwise for end of game 
 */
Error start_hub(Game* game, Session* session) {
    int signal = check_signal();
    if(signal) {
        return signal;
    }

    if(send_tokens(game->pCount, session->players, game->tokens[0]) != OK ||
            send_card(game, session, 8) != OK) { // pre-game setup
        return E_DEADPLAYER;
    }

    Error err = OK;
    while((signal = check_signal()), !signal && err == OK) {
        for(int i = 0; i < game->pCount; i++) {
            Msg response;
            Comm responseType = get_player_move(game, 
                    session->players[i], &response);
            if(responseType == PURCHASE) {

            } else if(responseType == TOOK) {

            } else if(responseType == WILD) {

            } else {
                free(response.info);
                return E_PROTOCOL;
            }

            if(signal = check_signal(), signal) {
                free(response.info);
                return signal;
            }
            
        }
        // if end of game reached, err = UTIL;
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
    printf("parent PID:\t%d\n", getpid());
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

#ifdef TEST
    printf("hub initialized, starting players\n");
#endif
    
    int signalList[] = {SIGINT, SIGPIPE, SIGCHLD};
    init_signal_handler(signalList, 3);

    if(start_players(argc - 4, argv + 4, &game, &session) != OK ||
            getpid() != session.parentPID) {
        end_game(&game, &session, E_EXEC);
    } // no child should go past here

#ifdef TEST
    printf("game starting\n");
#endif

    err = start_hub(&game, &session);
    end_game(&game, &session, err);
    
#ifdef TEST
    printf("eog reached\n");
#endif

    return err;
}
