#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "err.h"
#include "common.h"
/*
#include "comms.h"
#include "token.h"
#include "card.h"
*/

/*
 * checks contents of deck and saves to memory
 * params:  deck - file containing deck
 * returns: ERR if invalid contents,
 *          OK otherwise
 */
Err read_deck(FILE* deck) {
    Err error = OK;
    int numCards = 0;
    char* line = (char*)malloc(sizeof(char) * LINE_BUFF);
    while(1) {
        if(fgets(line, LINE_BUFF, deck) == NULL) {
            if(!numCards) {
                error = ERR;
            }

            break;
        }
       
        int res, color, points, purple, brown, yellow, red;
        res = sscanf(line, "%c:%d:%d,%d,%d,%d", 
                (char*)&color, &points, &purple, &brown, &yellow, &red);
        if(res != 6) {
            printf("got invalid line: %s", line);
            error = ERR;
            break;
        }
        
#ifdef TEST
        printf("got line: %s", line);
#endif

        numCards++;
    }

    free(line);

#ifdef TEST
    printf("got %d cards\n", numCards);
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
 * returns: E_ARGV if any invalid arguments, 
 *          E_DECKIO if deck cannot be accessed, 
 *          E_DECKR if invalid deck contents,
 *          E_EXEC if players couldn't be started
 *          OK otherwise
 */
Err init_game(int argc, char** argv) {
    char* temp;
    long int numTokens = strtol(argv[1], &temp, 10);
    long int numPoints = strtol(argv[2], &temp, 10);

    if(!numTokens || numTokens < 0 || !numPoints || numPoints < 0) {
        return E_ARGV;
    }

    FILE* deck = fopen(argv[3], "r");
    if(!deck) {
        return E_DECKIO;
    }

    if(read_deck(deck) != OK) {
        return E_DECKR;
    }
    fclose(deck);
    
    if(start_players(argc - 3, argv + 3) != OK) {
        return E_EXEC;
    }

    return OK;
}

int main(int argc, char** argv) {
    Err error = OK;
    if(argc < 6) {
        error = E_ARGC;
        herr_msg(error);
        return error;
    }

    error = init_game(argc, argv);
    if(error) {
        herr_msg(error);
        return error;
    }


    herr_msg(error);
    return error;
}
