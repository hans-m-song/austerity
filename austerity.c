#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <limits.h>
#include "err.h"
#include "common.h"
#include "card.h"
#include "comms.h"
#include "hub.h"
#include "signalHandler.h"

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
    int sig = check_signal();
    while((sig = check_signal()), sig == OK) {
        Msg msg;
        msg.type = EOG;
        broadcast(game->pCount, session->players, &msg);
        return OK;
    }
    
    // TODO game loop
    // do loop here probably
    // for(int i = 0; i < game->numCards, i < 8; i++)
    //      newcard
    return E_SIGINT;
}

/*
 * reaps player processes
 * params:  game - struct containing relevant game information
 */
void kill_players(int pCount, Player* players) {
    for(int i = 0; i < pCount; i++) {
#ifdef TEST
        printf("%d killing:\tplayer \t%d\n", getpid(), players[i].pid);
#endif
        
        int status = 0;
        dprintf(players[i].pipeOut[WRITE], "eog");
        if(!waitpid(players[i].pid, &status, WNOHANG)) {
            sleep(2);

            if(!status) { // force kill
#ifdef TEST
                printf("sending sigkill to %d\n", players[i].pid);
#endif

                kill(players[i].pid, SIGTERM); // or SIGKILL?
            }
        }
    }
}

/*
 * clears memory used by hub
 * params:  game - struct containing relevant game information
 *          session - struct containing hub only information
 *          err - code to exit with
 */
void end_game(Game* game, Session* session, Error err) {
#ifdef TEST
    printf("exit:\tgot code %d\n", err);
#endif

    shred_deck(game->stack.deck, game->stack.numCards);
    if(session->parentPID == getpid()) {
        kill_players(game->pCount, session->players);
    }
    for(int i = 0; i < game->pCount; i++) {
        close(session->players[i].pipeIn[READ]);
        close(session->players[i].pipeOut[WRITE]);
    }
    free(session->players);

    if(err) {
        herr_msg(err);
        exit(err);
    }
}

/*
 * sets up piping for parent or child
 * params:  player - struct containing player pipes
 *          type - char of either 'p': parent or 'c': child
 */
void pipe_setup(Player player, char type) {
    if(type == 'p') { // parent
        close(player.pipeIn[WRITE]);
        close(player.pipeOut[READ]);
    } else { // child
        dup2(player.pipeOut[READ], STDIN_FILENO);
        dup2(player.pipeIn[WRITE], STDOUT_FILENO);
        close(player.pipeIn[READ]);
        close(player.pipeIn[WRITE]);
        close(player.pipeOut[READ]);
        close(player.pipeOut[WRITE]);
    }
}

/*
 * starts the given players
 * params:  pCount - number of players to start
 *          players - array of player commands to exec
 *          game - struct containing relevant game information
 *          session - struct containing hub only information
 * returns: E_EXEC if any players did not start successfully,
 *          OK otherwise
 */
Error start_players(int pCount, char** players, 
        Game* game, Session* session) {
    session->players = (Player*)malloc(sizeof(Player) * pCount);
    for(int i = 0; i < pCount; i++, game->pCount++) {
        if(pipe(session->players[i].pipeIn) < 0 || 
                pipe(session->players[i].pipeOut) < 0) {
            return E_EXEC;
        }
        char* totalPlayers = to_string(pCount);
        char* pID = to_string(i);
        char* args[] = {players[i], totalPlayers, pID, NULL};

#ifdef TEST
        printf("exec:\t%s %s %s\n", args[0], args[1], args[2]);
#endif

        pid_t pid = fork();
        if(pid < 0) { // failed to fork
            return E_EXEC;
        } else if(pid == 0) { // child
            //pipe_setup(session->players[i], 'c');
            if(execvp(args[0], args) < 0) { // failed to exec
#ifdef TEST
                printf("failed to exec:\t%s\n", args[0]);
#endif
                free(totalPlayers);
                free(pID);

                return E_EXEC;
            }
        } else { // parent
#ifdef TEST
            printf("child:\t%d:%d\n", i, pid);
#endif

            session->players[i].pid = pid;
            //pipe_setup(session->players[i], 'p');
        }
        free(totalPlayers);
        free(pID);
    }

    return OK;
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
    
    int signalList[] = {SIGINT, SIGPIPE};
    init_signal_handler(signalList, 2);

    if(start_players(argc - 4, argv + 4, &game, &session) != OK ||
            getpid() != session.parentPID) {
        end_game(&game, &session, E_EXEC);
    } // no child should go past here

#ifdef TEST
    printf("started %d players\n", game.pCount);
#endif

    err = start_hub(&game, &session);
    
#ifdef TEST
    printf("reached end of game\n");
#endif

    end_game(&game, &session, err);
    return err;
}
