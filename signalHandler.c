#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include "err.h"
#include "signalHandler.h"

/*
 * global variable used to tell if a signal has been caught
 */
volatile sig_atomic_t globalSignal = 0;

/*
 * signal handler for catching signals, sets the globalSignal flag
 * params:  signal - numeric code of signal caught
 */
void signal_handler(int signal) {
#ifdef TEST
    fprintf(stdout, "[%d]signal %d caught\n", getpid(), signal);
#endif

    globalSignal = signal;
}

/*
 * resets the global signal flag
 */
void reset_signal(void) {
    globalSignal = 0;
}

/*
 * returns the current state of globalSignal
 * returns: E_SIGINT, E_DEADPLAYER based on the state of globalSignal
 */
int check_signal(void) {
    //return globalSignal;
    switch(globalSignal) {
        case SIGINT:
            return E_SIGINT;
        case SIGCHLD:
            return E_DEADPLAYER;
        case SIGPIPE:
            return E_DEADPLAYER;
        default:
            return OK;
    }
}

/*
 * registers the signal handler to the relevant signals
 * params:  signalList - signals to register
 *          len - number of signals
 */
void init_signal_handler(int signalList[], int len) {
    // only catch relevant signals
    for(int i = 0; i < len; i++) {
#ifdef TEST
        printf("register signal %d\n", signalList[i]);
#endif
        
        struct sigaction sigact;
        memset(&sigact, 0, sizeof(sigact));
        sigact.sa_handler = &signal_handler;
        sigact.sa_flags = SA_RESTART;
        sigaction(signalList[i], &sigact, NULL);
    }

}
