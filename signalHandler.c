#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include "signalHandler.h"

/*
 * global variables used to tell if a signal has been caught
 */
volatile sig_atomic_t globalSignal = 0;

/*
 * signal handler for catching signals, sets the globalSignal flag
 * params:  signal - numeric code of signal caught
 */
void signal_handler(int signal) {
#ifdef TEST
    fprintf(stdout, "signal %d caught\n", signal);
#endif

    globalSignal = signal;
}

/*
 * returns the current state of globalSignal
 * returns: current state of globalSignal
 */
int check_signal(void) {
    return globalSignal;
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
