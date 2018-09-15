#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <limits.h>
#include <signal.h>
#include "err.h"
#include "common.h"
#include "card.h"
#include "comms.h"
#include "hub_init.h"

static int sigint = 0;

void sigint_handler(int signal) {
    printf("signal %d caught\n", signal);
    sigint = 1;
}

/*
 * main logic for the hub
 * params:
 * returns: E_DEADPLAYER if client disconnects,
 *          E_PROTOCOL if client is being naughty
 *          E_SIGINT if sigint was caught
 *          OK otherwise for end of stack
 */
Error start_hub(Game* game, pid_t parentPID) {
    while(!sigint) {
        if(getpid() != parentPID) { // child
            exit(ERR);
        } else { // parent
            for(int i = 0; i < game->pCount; i++) {
                int status;
                wait(&status);
                if(status) {
                    printf("Player %c ended with status %d\n", 
                            (char)(i + 65), status);
                }
            }
        }
        // TODO game loop
        // do loop here probably
        // for(int i = 0; i < game->numCards, i < 8; i++)
        //      newcard
        return OK;
    }
    return E_SIGINT; 
}

/*
 * reaps player processes
 * params:  game - struct containing relevant game information
 */
void kill_players(int pCount, pid_t playerPID[]) {
    for(int i = 0; i < pCount; i++) {
#ifdef TEST
        printf("killing players\n");
#endif

        // send eog
        // wait 2 seconds
        // if not dead,
        kill(playerPID[i], SIGKILL);
        printf("Player %c shutdown after receiving signal %d", 
                (char)(i + 65), SIGINT);
    }
}

/*
 * starts the given players
 * params:  pCount - number of players to start
 *          players - array of player commands to exec
 *          game - struct containing relevant game information
 * returns: E_EXEC if any players did not start successfully,
 *          OK otherwise
 */
Error start_players(char** players, pid_t* playerPID, Game* game) {
    for(int i = 0; i < game->pCount; i++) {
        char* pCount = to_string(game->pCount);
        char* pID = to_string(i);
        char* args[] = {players[i], pCount, pID, NULL};
#ifdef TEST
        printf("exec:\t%s %s %s\n", args[0], args[1], args[2]);
#endif

        pid_t pid = fork();
        if(pid == ERR) { // failed to fork
            // TODO reap other players, use wait()?
            return E_EXEC;
        } else if(pid == 0) { // child
            // TODO set up pipes
            if(execvp(args[0], args) == ERR) { // failed to exec
                // check FD_CLOEXEC
                return E_EXEC;
            }
        } else { // parent
            playerPID[i] = pid;
        }
        free(pCount);
        free(pID);
    }

    return OK;
}


int main(int argc, char** argv) {
    signal(SIGINT, sigint_handler);
    Game game;
    if(argc < 6 || argc - 4 > MAX_PLAYERS) {
        herr_msg(E_ARGC);
        return E_ARGC;
    }

    Error err = OK;

    err = hub_init(argc, argv, &game);
    if(err) {
        herr_msg(err);
        return err;
    }

    pid_t parentPID = getpid();
    pid_t* playerPID = (pid_t*)malloc(sizeof(pid_t) * game.pCount);
    err = start_players(argv + 4, playerPID, &game);
    if(err) {
        kill_players(game.pCount, playerPID);
        herr_msg(E_EXEC);
        return E_EXEC;
    }

    err = start_hub(&game, parentPID);
    if(err) {
        kill_players(game.pCount, playerPID);
        herr_msg(err);
    }

    shred_deck(game.stack.deck, game.stack.numCards);
    free(playerPID);
    return err;
}
