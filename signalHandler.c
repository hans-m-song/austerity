#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "signalHandler.h"

/*
 * global variables used to tell if a signal has been caught
 */
int globalSignal = 0;

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
 */
void init_signal_handler(int signalList[], int len) {
    globalSignal = 0;

    // only catch relevant signals
    for(int i = 0; i < len; i++) {
#ifdef TEST
        printf("register signal %d\n", signalList[i]);
#endif
        
        signal(signalList[i], signal_handler);
    }

}
