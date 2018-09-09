#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "err.h"
#include "common.h"
#include "card.h"

/*
#include "comms.h"
#include "token.h"
*/


/*
 * checks contents of deck and saves to memory
 * params:  deckFile - file containing deck
 *          game - struct containing relevant game information
 * returns: ERR if invalid contents,
 *          OK otherwise
 */
Err read_deck(FILE* deckFile, Game* game) {
    Err error = OK;
    char* line = (char*)malloc(sizeof(char) * LINE_BUFF);
    while(1) {
        if(fgets(line, LINE_BUFF, deckFile) == NULL) {
            if(!game->numCards) {
                error = ERR;
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
            error = ERR;
            break;
        }
        
#ifdef TEST
        printf("got line: %s", line);
#endif

        if(add_card(game, color, points, 
                    purple, brown, yellow, red) != OK) {
            return ERR;
        }
        game->numCards++;
    }

    free(line);

#ifdef TEST
    printf("got %d cards\n", game->numCards);
#endif

    return error;
}

/*
 * starts the given players
 * params:  pCount - number of players to start
 *          players - array of player commands to exec
 * returns: ERR if any players did not start successfully,
 *          OK otherwise
 */
Err start_players(int pCount, char** players) {
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
Err init_game(int argc, char** argv, Game* game) {
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
        return E_DECKR;
    }
    fclose(deckFile);
    print_deck(game->deck, game->numCards);
    
    if(start_players(argc - 3, argv + 3) != OK) {
        return E_EXEC;
    }

    return OK;
}

int main(int argc, char** argv) {
    Err error = OK;
    Game game;
    if(argc < 6) {
        error = E_ARGC;
        herr_msg(error);
        return error;
    }

    error = init_game(argc, argv, &game);
    if(error) {
        shred_deck(game.deck, game.numCards);
        herr_msg(error);
        return error;
    }

    herr_msg(error);
    return error;
}
