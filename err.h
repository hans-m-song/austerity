#ifndef ERR_H
#define ERR_H

// error codes for hub and player
typedef enum {
    OK = 0,
    E_ARGC = 1,
    E_PCOUNT = 2,
    E_ARGV = 2,
    E_PID = 3,
    E_DECKIO = 3,
    E_DECKR = 4,
    E_EXEC = 5,
    E_PIPECLOSE = 6,
    E_DEADPLAYER = 6,
    E_PROTOCOL = 7,
    E_SIGINT = 10,
    ERR = -1 
} Err;

void perr_msg(Err code, char* ptype);

void herr_msg(Err code);

#endif
