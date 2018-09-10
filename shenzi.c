#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "err.h"
#include "playerCommon.h"

int main(int argc, char** argv) {
    if(argc != 3) {
        perr_msg(E_ARGC, "shenzi");
        return E_ARGC;
    }
    
    int pCount = check_pcount(argv[1]);
    if (pCount == ERR) {
        perr_msg(E_PCOUNT, "shenzi");
        return E_PCOUNT;
    }

    int pID = check_pid(argv[2], pCount);
    if(pID == ERR) {
        perr_msg(E_PID, "shenzi");
        return E_PID;
    }

#ifdef TEST
    printf("shenzi[%d] launched successfully\n", pID);
#endif

    Error err = OK;
    return err;
}
