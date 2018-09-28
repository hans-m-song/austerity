#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <limits.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "err.h"
#include "common.h"
#include "card.h"
#include "comms.h"
#include "hub.h"
#include "token.h"
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
 * creates the message to send 1 or many cards to players, 
 * moves card from hubstack to stack to keep track of active cards in game
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
 * checks if any players have won the game
 * params:  numPoints - number of points to win
 *          pCount - number of players
 *          playerStats - stats of players in game
 * returns: UTIL if there are winners
 *          OK otherwise
 */
Error check_win(int winningPoints, int pCount, Game* playerStats) {
    int winners = 0;
    char winList[MAX_PLAYERS] = {0};
    for(int i = 0; i < pCount; i++) {
#ifdef TEST
        printf("%c has %d points\n", i + TOCHAR, playerStats[i].numPoints);
#endif

        if(playerStats[i].numPoints >= winningPoints) {
            winList[winners++] = (char)(i + TOCHAR);
        }
    }

    if(winners > 0) { // if winners found
        fprintf(stdout, "Winner(s) ");
        for(int i = 0; i < winners; i++) {
            fprintf(stdout, "%c", winList[i]);
            if(i < winners - 1) {
                fprintf(stdout, ",");
            }
        }
        fprintf(stdout, "\n");

        return UTIL;
    }

    return OK;
}

/*
 * updates token amounts for game and player
 * params:  game - struct containing relevant game information
 *          playerStats - players current stats
 *          card - tokens used
 *          position - position of purchased card
 *          wild - wild tokens used
 */
void player_purchase(Game* game, Game* playerStats, Card card, 
        int position, int wild) {
    for(int i = 0; i < TOKEN_SIZE; i++) {
        game->tokens[i] += card[i + 2];
        playerStats->ownedTokens[i] -= card[i + 2];
    }
    int discountColor = 0;
    switch(card[COLOR]) {
        case 'B':
            discountColor = 1;
            break;
        case 'Y':
            discountColor = 2;
            break;
        case 'R':
            discountColor = 3;
    }
    playerStats->discount[discountColor] += 1;
    playerStats->numPoints += card[POINTS];
    playerStats->wild -= wild;
    fprintf(stdout, "Player %c purchased %d using %d,%d,%d,%d,%d\n", 
            playerStats->pID + TOCHAR, position,
            card[PURPLE], card[BROWN], card[YELLOW], card[RED], wild);
}

/*
 * checks if the players requested move is legal
 * params:  game - struct containing relevant game information
 *          playerStats - players current stats
 *          msg - contents of message to check for validity
 * returns: ERR if illegal,
 *          OK otherwise
 */
Error valid_move(Game* game, Game playerStats, Msg* msg) {
    if(msg->type == WILD) {
#ifdef TEST
        printf("%c requested wild\n", playerStats.pID + TOCHAR);
#endif
        return OK;
    } 
    
    if(msg->type == TAKE) {
#ifdef TEST
        printf("%c requested tokens:%d,%d,%d,%d; have:%d,%d,%d,%d\n", 
                playerStats.pID + TOCHAR, 
                msg->info[PURPLE], msg->info[BROWN], msg->info[YELLOW], 
                msg->info[RED], game->tokens[0], game->tokens[1],
                game->tokens[2], game->tokens[3]);
#endif
        if(sum_tokens(msg->info, game->discount) > 3) {
            return ERR;
        }

        for(int i = 0; i < TOKEN_SIZE; i++) {
            if(msg->info[i + 2] != 1 && msg->info[i + 2] != 0) {
                return ERR;
            }
            
            if(msg->info[i + 2] == 1 && game->tokens[i] < 1) {
                return ERR;
            }
        }

        return OK;
    } 
    
    if(msg->type == PURCHASE) {
#ifdef TEST
        fprintf(stderr, "%c requested card: ", playerStats.pID + TOCHAR);
        print_card(game->stack.deck[msg->card], msg->card);
#endif
        int wild = can_afford(msg->info, playerStats.discount, 
                playerStats.ownedTokens, playerStats.wild);
        if(wild < 0 || (wild > 0 && playerStats.wild < wild)) {
            return ERR;
        }

        return OK;
    }

    return ERR;
}

/*
 * execute player move, assumes move is legal
 * params:  game - struct containing relevant game information
 *          session - struct containing hub only information
 *          response - struct message contents is saved to
 *          pID - player whos move is being executed
 * returns: E_DEADPLAYER if broadcast fails,
 *          OK otherwise
 */
Error do_move(Game* game, Session* session, Msg* response, int pID) {
    Msg alert = {-1, 0, 0, 0, 0, 0};
    alert.info = (Card)calloc(CARD_SIZE, sizeof(int));
    if(response->type == WILD) {
        alert.type = WILD;
        session->playerStats[pID].wild++;
        fprintf(stdout, "Player %c took a wild\n", pID + TOCHAR); 
    }

    if(response->type == TAKE) {
        alert.type = TOOK;
        for(int i = 0; i < TOKEN_SIZE; i++) {
            if(response->info[i + 2]) {
                game->tokens[i]--;
                session->playerStats[pID].ownedTokens[i]++;
                alert.info[i + 2]++;  
            }
        }
#ifdef TEST
        printf("bank now:%d,%d,%d,%d\n",
                game->tokens[0], game->tokens[1], 
                game->tokens[2], game->tokens[3]);
#endif

        fprintf(stdout, "Player %c drew %d,%d,%d,%d\n", pID + TOCHAR,
                response->info[PURPLE], response->info[BROWN],
                response->info[YELLOW], response->info[RED]);
    }

    if(response->type == PURCHASE) {
        alert.type = PURCHASED;
        memcpy(alert.info, response->info, CARD_SIZE * sizeof(int));
        alert.card = response->card;
        alert.wild = response->wild;
        alert.info[POINTS] = game->stack.deck[alert.card][POINTS];
        player_purchase(game, &session->playerStats[pID], 
                alert.info, alert.card, alert.wild);
        remove_card(&game->stack, alert.card);
    }

    alert.player = (char)(pID + TOCHAR);
    Error err = broadcast(game->pCount, session->players, &alert);
    free(alert.info);
    free(response->info);
    return err;
}

/*
 * asks player for their next move, reprompt on invalid move
 * params:  game - struct containing relevant game information
 *          player - player to retrieve response from
 *          playerStats - players current stats
 *          response - struct to save message contents to
 * returns: type of message received
 *          ERR if error encountered or reprompt returns invalid move
 */
Comm get_player_move(Game* game, Player player, Game playerStats, 
        Msg* response) {
#ifdef TEST
    fprintf(stderr, "%c stats; points:%d; wild:%d; tokens:%d,%d,%d,%d; "
            "discount%d,%d,%d,%d\n", playerStats.pID + TOCHAR, 
            playerStats.numPoints, playerStats.wild,
            playerStats.ownedTokens[0], playerStats.ownedTokens[1],
            playerStats.ownedTokens[2], playerStats.ownedTokens[3],
            playerStats.discount[0], playerStats.discount[1],
            playerStats.discount[2], playerStats.discount[3]);
#endif
    Msg request = {DOWHAT, 0, 0, 0, 0, 0};
    response->info = (Card)malloc(sizeof(int) * CARD_SIZE);
    char* line;
    for(int i = 0; i < 2; i++) {
#ifdef TEST
        if(i) {
            printf("reprompting\n");
        }
#endif

        send_msg(&request, player.toChild);
        line = read_line(player.fromChild, 0, 0);

#ifdef VERBOSE 
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
                valid_move(game, playerStats, response) == OK) {
            return response->type;
        }
    }
    
    free(response->info);
    return ERR;
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
            if((int)get_player_move(game, session->players[i], // get move
                    session->playerStats[i], &response) == ERR) {
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
            
            err = do_move(game, session, &response, i); // execute move
            if(!err && response.type == PURCHASE && 
                    game->hubStack.numCards > 0) { // if card was bought
                err = send_card(game, session, 1);
            }
        }
        if(!err) { // check for winners
            err = check_win(game->numPoints, game->pCount, 
                    session->playerStats);
        }
    }

    if(err == UTIL) {
        Msg endGame = {EOG, 0, 0, 0, 0, 0};
        err = broadcast(game->pCount, session->players, &endGame);
        return err;
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
