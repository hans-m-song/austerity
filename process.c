#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "err.h"
#include "common.h"
#include "card.h"
#include "comms.h"
#include "hub.h"
#include "signalHandler.h"

/*
 * reaps player processes
 * params:  game - struct containing relevant game information
 *          err - if an error caused hub to kill players
 */
void kill_players(int pCount, Player* players, Error err) {
    for(int i = 0; i < pCount; i++) {
        if(players[i].toChild != NULL) {
            fclose(players[i].toChild);
            fclose(players[i].fromChild);
        }

#ifdef TEST
        fprintf(stderr, "[%d]killing: player %c:%d, err:%d\n", 
                getpid(), i + TOCHAR, players[i].pid, err);
#endif
        
        if(err) {
            kill(players[i].pid, SIGKILL);
            continue;
        }
            
        int status = 0;
        dprintf(players[i].pipeOut[WRITE], "eog\n");
        sleep(2);
        if(!waitpid(players[i].pid, &status, WNOHANG)) {
            kill(players[i].pid, SIGKILL);
            fprintf(stderr, "Player %c shutdown after receiving signal %d\n", 
                    i + TOCHAR, SIGKILL);
            continue;
        }

        if(WIFEXITED(status) && WEXITSTATUS(status)) {
            fprintf(stderr, "Player %c ended with status %d\n", 
                    i + TOCHAR, WEXITSTATUS(status));
            continue;
        }

        if(WIFSIGNALED(status)) {
            fprintf(stderr, "Player %c shutdown after receiving signal %d\n", 
                    i + TOCHAR, WTERMSIG(status));
            continue;
        }
    }
}

/*
 * sets up piping for parent or child
 * params:  player - struct containing player pipes
 *          type - char of either 'p': parent or 'c': child
 */
void pipe_setup(Player* player, char type) {
    if(type == 'p') { // parent
        close(player->pipeIn[WRITE]);
        close(player->pipeOut[READ]);
    } else { // child
        dup2(player->pipeOut[READ], STDIN_FILENO);
        dup2(player->pipeIn[WRITE], STDOUT_FILENO);
        close(player->pipeIn[READ]);
        close(player->pipeIn[WRITE]);
        close(player->pipeOut[READ]);
        close(player->pipeOut[WRITE]);
    }
}

/*
 * starts pipes for players
 * params:  player - player to initialize pipes for
 * returns: E_EXEC if pipe failed,
 *          OK otherwise
 */
Error init_pipe(Player* player) {
    if(pipe(player->pipeIn) < 0) {
        return E_EXEC;
    }

    if(pipe(player->pipeOut) < 0) {
        close(player->pipeIn[READ]);
        close(player->pipeIn[WRITE]);
        return E_EXEC;
    }

    return OK;
}

/*
 * makes the exec parameters and execs
 * params:  pCount - number of players
 *          pID - this players id
 *          players - function to exec
 * returns: should not return anything
 *          E_EXEC if exec failed
 */
Error make_exec(int pCount, int pID, char** players) {
    char* totalPlayers = to_string(pCount);
    char* playerNum = to_string(pID);
    char* args[] = {players[pID], totalPlayers, playerNum, NULL};

#ifdef TEST
    fprintf(stderr, "exec:\t%s %s %s\n", args[0], args[1], args[2]);
#endif

    execvp(args[0], args);
    free(totalPlayers);
    free(playerNum);
    return E_EXEC;
}

/*
 * checks if all players started successfully, opens files if so
 * params:  pCount - number of players to start
 *          session - struct containing hub only information
 * returns: E_EXEC if players failed,
 *          OK otherwise
 */
Error check_player_start(int pCount, Session* session) {
    sleep(1); // wait for failed exec to return
    for(int i = 0; i < pCount; i++) { // check if exec succeeded
        int status = 0;
        int returnPID = waitpid(session->players[i].pid, &status, WNOHANG);
        if(returnPID != 0) {
#ifdef TEST
            printf("%c exited prematurely, status:%d\n", 
                    i + TOCHAR, WEXITSTATUS(status));
#endif
            session->players[i].toChild = NULL;
            session->players[i].fromChild = NULL;
            return E_EXEC;
        }
        
        session->players[i].toChild = 
                fdopen(session->players[i].pipeOut[WRITE], "w");
        session->players[i].fromChild = 
                fdopen(session->players[i].pipeIn[READ], "r");

#ifdef TEST
        char* line = read_line(session->players[i].fromChild, 1, 1);
        printf("child %c announcing: %s", i + TOCHAR, line);
        free(line);
#endif
    }

    return OK;
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
Error start_players(int pCount, char** players, Game* game, Session* session) {
    session->parentPID = getpid();
    session->playerStats = (Game*)malloc(sizeof(Game) * pCount);
    session->players = (Player*)malloc(sizeof(Player) * pCount);
    for(int i = 0; i < pCount; i++, game->pCount++) {
        if(init_pipe(&session->players[i]) != OK) {
            return E_EXEC;
        }

        session->playerStats[i].pID = i;
        session->playerStats[i].numPoints = 0;
        memset(session->playerStats[i].discount, 
                0, sizeof(int) * TOKEN_SIZE);
        memset(session->playerStats[i].ownedTokens, 
                0, sizeof(int) * TOKEN_SIZE);
        session->playerStats[i].wild = 0;

        pid_t pid = fork();
        
        if(pid < 0) { // failed to fork
            return E_EXEC;
        }
        
        if(pid == 0) { // child
            pipe_setup(&session->players[i], 'c');
#ifdef TEST
            printf("child %d started\n", getpid());
            fflush(stdout);
#endif
            return make_exec(pCount, i, players); // should not return
        } 
        
        if(pid > 0) { // parent
            session->players[i].pid = pid;
            pipe_setup(&session->players[i], 'p');
        }
    }

    return check_player_start(pCount, session);
}

