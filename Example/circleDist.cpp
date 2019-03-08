#include "BrickPi3.h" // for BrickPi3
#include <iostream>      // for cout
#include <unistd.h>     // for sleep
#include <signal.h>     // for catching exit signals
#include <iomanip>		// for setw and setprecision

using namespace std;

BrickPi3 BP;

void exit_signal_handler(int signo);

void stop(void){
    BP.set_motor_power(PORT_B, 0);
    BP.set_motor_power(PORT_C, 0);
}

// De robot rijdt in een vierkant, nog niet getest.
void square(void)
{
    for(unsigned int i=0;i<4;i++)
    { // Rij vooruit
      
        BP.set_motor_power(PORT_B, 60);
        BP.set_motor_power(PORT_C, 56);
        sleep(2);
        // ga naar rechts
        BP.set_motor_dps(PORT_B, -45);
        BP.set_motor_dps(PORT_C, 45);
        usleep(700000);
        stop();
    }
}
// De robot rijdt in een cirkel, nog niet getest, niet af
void circle(void)
{
    for(unsigned int i=0;i<4;i++)
    {

        BP.set_motor_power(PORT_B, 50);
        BP.set_motor_power(PORT_C, 25);
        usleep(500000);
        //BP.set_motor_power(PORT_B, -270);
        //BP.set_motor_power(PORT_C, 270);
        sleep(2);
        stop();
    }
}

void fwd(void){
    BP.set_motor_power(PORT_B, 60);
    BP.set_motor_power(PORT_C, 56);
}

void left(void){
    BP.set_motor_dps(PORT_B, 45);
    BP.set_motor_dps(PORT_C, -45);
}

void right(void){
    BP.set_motor_dps(PORT_B, -45);
    BP.set_motor_dps(PORT_C, 45);
}

int main(){
  char inp;
  signal(SIGINT, exit_signal_handler); // register the exit function for Ctrl+C
  BP.detect(); // Make sure that the BrickPi3 is communicating and that the firmware is compatible with the drivers.
  int error;
  BP.set_sensor_type(PORT_2, SENSOR_TYPE_NXT_ULTRASONIC);
  BP.set_motor_limits(PORT_B, 60, 0);
  BP.set_motor_limits(PORT_C, 60, 0);
  sensor_ultrasonic_t Ultrasonic2;
  square();
  circle();
  fwd();
  while(true){
    error = 0;
    if(BP.get_sensor(PORT_2, Ultrasonic2) == 0 && Ultrasonic2.cm < 25){
      left();
      sleep(1);
      fwd();
    }
    sleep(0.5);
  }
}

// Signal handler that will be called when Ctrl+C is pressed to stop the program
void exit_signal_handler(int signo){
  if(signo == SIGINT){
    BP.reset_all();    // Reset everything so there are no run-away motors
    exit(-2);
  }
}
