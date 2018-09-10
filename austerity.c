#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "err.h"
#include "common.h"
#include "card.h"
#include "comms.h"

Error play_game(pid_t parentPID, Game* game) {
    if(getpid() != parentPID) {
        int status;
        wait(&status);
        printf("child playing game ended with %d\n", status);
        return status;
    }
    printf("parent playing game\n");
    return OK;
}

/*
#include "token.h"
*/

/*
 * checks contents of deck and saves to memory
 * params:  deckFile - file containing deck
 *          game - struct containing relevant game information
 * returns: ERR if invalid contents,
 *          OK otherwise
 */
Error read_deck(FILE* deckFile, Game* game) {
    Error err = OK;
    char* line = (char*)malloc(sizeof(char) * LINE_BUFF);
    while(1) {
        if(fgets(line, LINE_BUFF, deckFile) == NULL) {
            if(!game->numCards) {
                err = ERR;
            }
            break;
        }
      
        char color, end;
        int res, points, purple, brown, yellow, red;
        res = sscanf(line, "%c:%d:%d,%d,%d,%d%c", 
                &color, &points, &purple, &brown, &yellow, &red, &end);
        char* validColors = "PBYR";
        if(res != 7 || end != '\n' || strspn(&color, validColors) != 1 || 
                points < 0 || purple < 0 || brown < 0 || yellow < 0 || 
                red < 0) {
            err = ERR;
            break;
        }
        
#ifdef VERBOSE 
        printf("fgets:\t%s", line);
#endif

        if(add_card(game, color, points, 
                purple, brown, yellow, red) != OK) {
            return ERR;
        }
    }
    free(line);

#ifdef TEST
    printf("got:\t%d cards\n", game->numCards);
#endif

    return err;
}

/*
 * starts the given players
 * params:  pCount - number of players to start
 *          players - array of player commands to exec
 * returns: E_EXEC if any players did not start successfully,
 *          OK otherwise
 */
Error start_players(int pCount, char** players) {
    for(int i = 0; i < pCount; i++) {
        char* args[] = {players[i], to_string(pCount), to_string(i), NULL};
#ifdef TEST
        printf("exec:\t%s %s %s\n", args[0], args[1], args[2]);
#endif

        int pid = fork();
        if(pid == ERR) { // failed to fork
            // reap other players
            return E_EXEC;
        } else if(pid == 0) { // child
            if(execv(args[0], args) == ERR) { // failed to exec
                return E_EXEC;
            }
        }
    }
    return OK;
}

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
Error init_game(int argc, char** argv, Game* game) {
    char* temp;
    long int numTokens = strtol(argv[1], &temp, 10);
    long int numPoints = strtol(argv[2], &temp, 10);

    if(!numTokens || numTokens < 0 || !numPoints || numPoints < 0) {
        return E_ARGV;
    }

    FILE* deckFile = fopen(argv[3], "r");
    if(!deckFile) {
        return E_DECKIO;
    }

    game->numCards = 0;
    if(read_deck(deckFile, game) != OK) {
        shred_deck(game->deck, game->numCards);
        return E_DECKR;
    }
    fclose(deckFile);

#ifdef TEST
    print_deck(game->deck, game->numCards);
#endif
    
    return OK;
}

int main(int argc, char** argv) {
    Game game;
    if(argc < 6) {
        herr_msg(E_ARGC);
        return E_ARGC;
    }

    Error err = OK;

    err = init_game(argc, argv, &game);
    if(err) {
        herr_msg(err);
        return err;
    }

    pid_t parentPID = getpid();
    err = start_players(argc - 4, argv + 4);
    if(err) {
        herr_msg(E_EXEC);
        return E_EXEC;
    }

    err = play_game(parentPID, &game);

    shred_deck(game.deck, game.numCards);
    herr_msg(err);
    return err;
}
