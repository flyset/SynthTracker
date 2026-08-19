#ifndef AUDIO_H
#define AUDIO_H

#include <signal.h>

extern volatile sig_atomic_t stop;

void inthand(int signum);

#endif // AUDIO_H
