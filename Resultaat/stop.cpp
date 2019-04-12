//bestaat omdat niet alle programma's de motoren goed stoppen, vooral na een crash, dit programma kan makkelijk aangeroepen worden
//om de motoren te stoppen

#include "BrickPi3.h" // for BrickPi3
#include <stdio.h>      // for printf
#include <unistd.h>     // for usleep
#include <signal.h>     // for catching exit signals

BrickPi3 BP;


int main(){
    BP.reset_all();    // Reset everything so there are no run-away motors
    BP.reset_all();    // Reset everything so there are no run-away motors
	exit(-1);

}
