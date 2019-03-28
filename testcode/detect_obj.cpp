#include <unistd.h>     	// for sleep
#include <signal.h>     	// for catching exit signals
#include <iomanip>		// for setw and setprecision
#include <chrono>    		// To count elapsed time

using namespace std;

BrickPi3 BP;

void exit_signal_handler(int signo);

bool initialize(){
    BP.set_sensor_type(PORT_1, SENSOR_TYPE_NXT_COLOR_FULL);
    BP.set_sensor_type(PORT_2, SENSOR_TYPE_NXT_ULTRASONIC);
    BP.set_sensor_type(PORT_3, SENSOR_TYPE_NXT_LIGHT_ON);
    BP.set_sensor_type(PORT_4, SENSOR_TYPE_TOUCH);
    BP.set_sensor_type(PORT_2, SENSOR_TYPE_NXT_ULTRASONIC);
    BP.set_motor_limits(PORT_B, 60, 0);
    BP.set_motor_limits(PORT_C, 60, 0);
}

void stop(void){
    BP.set_motor_power(PORT_B, 0);
    BP.set_motor_power(PORT_C, 0);
}

void fwd(int speed=45){
    BP.set_motor_power(PORT_B, speed*1.07); //ivm ongelijkheid motoren.
    BP.set_motor_power(PORT_C, speed);
}

void detect(sensor_ultrasonic_t Ultrasonic2){
    while(true){
        if(BP.get_sensor(PORT_2, Ultrasonic2) == 0 && Ultrasonic2.cm < 6){
            stop();
        }
         else{
            fwd();
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
        sensor_color_t      Color1;
        sensor_ultrasonic_t Ultrasonic2;
        sensor_light_t      Light3;
        sensor_touch_t      Touch4;
        detect();


    }

// Signal handler that will be called when Ctrl+C is pressed to stop the program
    void exit_signal_handler(int signo){
        if(signo == SIGINT){
            BP.reset_all();    // Reset everything so there are no run-away motors
            exit(-2);
        }
    }
