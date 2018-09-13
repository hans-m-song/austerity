#include <stdio.h>
#include <stdlib.h>
#include "err.h"

/*
 * prints the corresponding player error code to stderr
 * params:  code - a number corresponding to an game ending error
 *          ptype - a string representing the type of player
 */
void perr_msg(Error code, int pType) {
    char* players[] = {"shenzi", "banzai", "ed"};
    switch(code) {
        case OK:
            return;
        case E_ARGC:
            fprintf(stderr, "Usage: %s pcount myid\n", players[pType]);
            break;
        case E_PCOUNT:
            fprintf(stderr, "Invalid player count\n");
            break;
        case E_PID:
            fprintf(stderr, "Invalid player ID\n");
            break;
        case E_COMMERR:
            fprintf(stderr, "Communication Error\n");
            break;
        default:
#ifdef TEST
            fprintf(stderr, "got invalid code: %d\n", code);
#endif
            break;
    }
}

/*
 * prints the corresponding hub error code to stderr
 * params:  code - a number corresponding to an game ending error
 */
void herr_msg(Error code) {
    char* str = "Usage: austerity tokens points "
            "deck player player [player ...]\n";
    switch(code) {
        case OK:
            return;
        case E_ARGC:
            fprintf(stderr, str);
            break;
        case E_ARGV:
            fprintf(stderr, "Bad argument\n");
            break;
        case E_DECKIO:
            fprintf(stderr, "Cannot access deck file\n");
            break;
        case E_DECKR:
            fprintf(stderr, "Invalid deck file contents\n");
            break;
        case E_EXEC:
            fprintf(stderr, "Bad start\n");
            break;
        case E_DEADPLAYER:
            fprintf(stderr, "Client disconnected\n");
            break;
        case E_PROTOCOL:
            fprintf(stderr, "Protocol error by client\n");
            break;
        case E_SIGINT:
            fprintf(stderr, "SIGINT caught\n");
            break;
        default:
#ifdef TEST
            fprintf(stderr, "got invalid code: %d\n", code);
#endif
            break;
    }
}

