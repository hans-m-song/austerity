#ifndef COMMS_H
#define COMMS_H

typedef enum {
    eog,
    dowhat,
    tokensT,
    newcard,
    purchased,
    took,
    wild
} Comm;

int send_msg(char* msg);

#endif
