#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <limits.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
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
    fprintf(stderr, "[%d]exit:\tgot code %d\n", getpid(), err);
#endif

    shred_deck(game->stack.deck, game->stack.numCards);
    shred_deck(game->hubStack.deck, game->hubStack.numCards);

    if(session->parentPID == getpid()) {
        kill_players(game->pCount, session->players, err);
        herr_msg(err);
    }
    
    free(session->players);
    free(session->playerStats);
    
    if(err) {
        exit(err);
    }
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
    char* line;
    for(int i = 0; i < 2; i++) {
        send_msg(&request, player.toChild);
        line = read_line(player.fromChild, 0, 0);

#ifdef TEST
        fprintf(stderr, "got line: %s\n", line);
#endif

        if(line == NULL || check_signal()) {
#ifdef TEST
            fprintf(stderr, "interrrupted by signal: %d, line:%s\n",
                    check_signal(), line);
#endif

            free(line);
            free(response->info);
            return ERR;   
        }

        if((int)decode_player_msg(response, line) != ERR &&
                valid_move(game, response) == OK) {
            return response->type;
        }
    }
    
    free(response->info);
    return ERR;
}

/*
 * checks if any players have won the game
 * params:  numPoints - number of points to win
 *          pCount - number of players
 *          playerStats - stats of players in game
 * returns: UTIL if there are winners
 *          OK otherwise
 */
Error check_win(int winningPoints, int pCount, Game* playerStats) {
    for(int i = 0; i < pCount; i++) {
        if(playerStats[i].numPoints >= winningPoints) {
            return UTIL;
        }
    }
    return OK;
}

/*
 * execute player move
 * params:  game - struct containing relevant game information
 *          session - struct containing hub only information
 *          response - struct message contents is saved to
 */
Error do_move() { //Game* game, Session* session, Msg* response) {
    return OK;
}

/*
 * main logic for the hub;
 * for all players: request move (reprompt once if necessary)
 *                  execute move
 * check if any players have reached the win condition
 * repeat
 * params:  game - struct containing relevant game information
 *          session - struct containing hub only information
 * returns: E_DEADPLAYER if client disconnects,
 *          E_PROTOCOL if client is being naughty
 *          E_SIGINT if sigint was caught
 *          UTIL otherwise for end of game 
 */
Error start_hub(Game* game, Session* session) {
    if(send_tokens(game->pCount, session->players, game->tokens[0]) != OK ||
            send_card(game, session, 8) != OK) { // pre-game setup
        return E_DEADPLAYER;
    }

    int signal;
    Error err = OK;
    while((signal = check_signal()), !signal && err == OK) {
        for(int i = 0; i < game->pCount; i++) {
            Msg response = {-1, 0, 0, 0, 0, 0};
            if((int)get_player_move(game, session->players[i], 
                    &response) == ERR) {
                signal = check_signal();
                if(errno == EINTR || signal == E_SIGINT) {
                    err = E_SIGINT;
                } else if(signal == E_DEADPLAYER) {
                    err = E_DEADPLAYER;
                } else {
                    err = E_PROTOCOL;
                }
                break;
            }
            
            err = do_move(game, session, &response);
        }
        if(!err) {
            printf("check win\n");
            err = check_win(game->numPoints, game->pCount, 
                    session->playerStats);
        }
    }

    if(err == UTIL) {
        Msg endGame = {EOG, 0, 0, 0, 0, 0};
        err = broadcast(game->pCount, session->players, &endGame);
    }

    if(!err) {
        return signal;
    }

    return err;
}

int main(int argc, char** argv) {
    Game game;
    Session session;

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
    printf("hub initialized, params:tokens=%d, points=%d, starting players\n",
            game.tokens[0], game.numPoints);
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
