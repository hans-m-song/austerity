#include <stdio.h>
#include <stdlib.h>
#include "err.h"

/*
 * takes the message and pipes it through the given fd
 * params:  pipe - fd to send message to
 *          msg - message to send
 * returns: E_PIPECLOSE, E_PROTOCOL, ERR, OK
 */
int send_msg(char* msg) {
    printf("sending message %s\n", msg);
    return OK;
}
