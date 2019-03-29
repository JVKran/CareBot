#include "BrickPi3.h" 		// for BrickPi3
#include <iostream>
#include <unistd.h>     	// for sleep
#include <signal.h>     	// for catching exit signals
#include <iomanip>		// for setw and setprecision
#include <chrono>    		// To count elapsed time

using namespace std;

BrickPi3 BP;

void exit_signal_handler(int signo);

bool initialize(){
    BP.set_sensor_type(PORT_2, SENSOR_TYPE_NXT_ULTRASONIC);
}

//functie die test of de Ultrasonic sensor goed werkt
void detect(sensor_ultrasonic_t Ultrasonic2){
    BP.get_sensor(PORT_2, Ultrasonic2);
    while(true){
        usleep(200000);
        if(BP.get_sensor(PORT_2, Ultrasonic2) == 0){
            BP.get_sensor(PORT_2, Ultrasonic2);
            cout << "Afstand: " << Ultrasonic2.cm << " cm" << endl;
        }
    }
}

int main(){
    signal(SIGINT, exit_signal_handler); // register the exit function for Ctrl+C
    BP.detect(); // Make sure that the BrickPi3 is communicating and that the firmware is compatible with the drivers.

    if(!initialize()){
        cout << "Initialisatie gelukt!";
    } else {
        cout << "Initialisatie mislukt...";
        BP.reset_all();
        exit(-2);
    }

    sensor_ultrasonic_t Ultrasonic2;

    detect(Ultrasonic2);


}

// Signal handler that will be called when Ctrl+C is pressed to stop the program
void exit_signal_handler(int signo){
    if(signo == SIGINT){
        BP.reset_all();    // Reset everything so there are no run-away motors
        exit(-2);
    }
}