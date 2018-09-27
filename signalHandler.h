#ifndef SIGNAL_HANDLER_H
#define SIGNAL_HANDLER_H

void signal_handler(int signal);

int check_signal(void);

void reset_signal(void);

void init_signal_handler(int signalList[], int len);

#endif
