

#include "BrickPi3.h" // for BrickPi3
#include <stdio.h>      // for printf
#include <unistd.h>     // for usleep
#include <signal.h>     // for catching exit signals

BrickPi3 BP;

void exit_signal_handler(int signo);

int main(){
    BP.reset_all();    // Reset everything so there are no run-away motors
    BP.reset_all();    // Reset everything so there are no run-away motors
	exit(-1);

}
