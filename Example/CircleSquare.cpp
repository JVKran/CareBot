#include "BrickPi3.h"   // for BrickPi3
#include <iostream>      // for printf
#include <unistd.h>     // for usleep
#include <signal.h>     // for catching exit signals

using namespace std;

BrickPi3 BP;

void exit_signal_handler(int signo);

//Stop
void stop(void)
{
    BP.set_motor_power(PORT_B, 0);
    BP.set_motor_power(PORT_C, 0);
}
// De robot rijdt in een vierkant, nog niet getest.
void square(void)
{
    for(unsigned int i=0;i<4;i++)
    {
        BP.set_motor_power(PORT_B, 20);
        BP.set_motor_power(PORT_C, 20);
        usleep(500000);
        BP.set_motor_power(PORT_B, 40);
        BP.set_motor_power(PORT_C, 40);
        usleep(500000);
        BP.set_motor_power(PORT_B, 50);
        BP.set_motor_power(PORT_C, 50);
        usleep(500000);
        BP.set_motor_power(PORT_B, -270);
        BP.set_motor_power(PORT_C, 270);
        sleep(2);
        stop();
    }
}
// De robot rijdt in een cirkel, nog niet getest, niet af
void circle(void)
{
    for(unsigned int i=0;i<4;i++)
    {

        BP.set_motor_power(PORT_B, 50);
        BP.set_motor_power(PORT_C, 50);
        usleep(500000);
        BP.set_motor_power(PORT_B, -270);
        BP.set_motor_power(PORT_C, 270);
        sleep(2);
        stop();
    }
}



//Move Forward
void fwd(void)
{
    BP.set_motor_power(PORT_B, 20);
    BP.set_motor_power(PORT_C, 20);
    usleep(500000);
    BP.set_motor_power(PORT_B, 40);
    BP.set_motor_power(PORT_C, 40);
    usleep(500000);
    BP.set_motor_power(PORT_B, 50);
    BP.set_motor_power(PORT_C, 50);
    sleep(2);
    stop();

}
//Move Left
void left(void)
{
    BP.set_motor_position_relative(PORT_B, 270);
    BP.set_motor_position_relative(PORT_C, -270);
}
//Move Right
void right(void)
{
    BP.set_motor_position_relative(PORT_B, -270);
    BP.set_motor_position_relative(PORT_C, 270);
}
//Move backward
void back(void)
{
    BP.set_motor_dps(PORT_B, -360);
    BP.set_motor_dps(PORT_C, -360);
    sleep(3);
    stop();
}

int main()
{
    signal(SIGINT, exit_signal_handler); // register the exit function for Ctrl+C
    BP.detect(); // Make sure that the BrickPi3 is communicating and that the firmware is compatible with the drivers.
    BP.set_motor_limits(PORT_B, 60, 0);
    BP.set_motor_limits(PORT_C, 60, 0);
    char inp;

    while(true)
    { //Heb de inputs veranderd naar wasd en om een vierkant te maken, moet je z in drukken en voor cirkel r
        cout << "Press w (forward), s (backward), a(left), d (right), z (square), r (circle) x (stop): " << endl;
        cin >> inp;	//Take input from the terminal
        //Move the bot
        if(inp=='f') {
            fwd();
        }

        else if (inp=='z') {
            square();
        }
        else if (inp=='r'){
            circle();
        }

        else if (inp=='s') {
            back();
        }

        else if (inp=='a'){
            left();
        }
        else if (inp=='d'){
            right();
        }
        else if (inp=='x'){
            stop();
        }

    }

    return 0;
}

// Signal handler that will be called when Ctrl+C is pressed to stop the program
void exit_signal_handler(int signo){
    if(signo == SIGINT){
        BP.reset_all();    // Reset everything so there are no run-away motors
        exit(-2);
    }
}
