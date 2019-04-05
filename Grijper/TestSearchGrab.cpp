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

    if(BP.get_voltage_battery() < 10.85){
        cout << "Batterijspanning is te laag! Namelijk " << BP.get_voltage_battery() << "V. Script wordt getermineerd." << endl << endl;
        BP.reset_all();
        exit(-2);
    } else {
        cout << "Batterijspanning is goed. Namelijk " << BP.get_voltage_battery() << "V." << endl << endl;
    }
    BP.set_sensor_type(PORT_2, SENSOR_TYPE_NXT_ULTRASONIC);
    BP.set_motor_limits(PORT_D, 20, 0);
    BP.set_motor_limits(PORT_B, 20, 0);
    BP.set_motor_limits(PORT_C, 20, 0);
}

void fwd(int speed=45){
    BP.set_motor_power(PORT_B, speed*1.07); //ivm ongelijkheid motoren.
    BP.set_motor_power(PORT_C, speed);
}

void stop(void){
    BP.set_motor_power(PORT_B, 0);
    BP.set_motor_power(PORT_C, 0);
}

void left(int speed=45){
    BP.set_motor_dps(PORT_B, speed*1.07);
    BP.set_motor_dps(PORT_C, -speed);
}

void right(int speed=45){
    BP.set_motor_dps(PORT_B, -speed*1.07);
    BP.set_motor_dps(PORT_C, speed);
}

/* de functie detect krijgt als parameter de Ultrasonic sensor mee om te checken of er een voorwerp
 * ongeveer 50 cm voor de robot staat, als er een voorwerp is maar die is te ver om te grijpen rijd
 * de robot er heen als het recht voor de sensor staat en als het voorwerp te verweg is voor beide
 * stopt het de loop.
*/
void detect(sensor_ultrasonic_t Ultrasonic2){
    //zet de encoder
    BP.offset_motor_encoder(PORT_D, BP.get_motor_encoder(PORT_D));
    int32_t EncoderD = BP.get_motor_encoder(PORT_D);
    BP.set_motor_position(PORT_D, EncoderD);

    BP.set_motor_position(PORT_D, -4);		//De klauwen gaan open
    cout << "De klauwen worden nu geopend!" << endl;

    while(true){
        usleep(50000);      //slaap voor een korte tijd om betrouwbaardere waardes uit te lezen

        //checked of er een voorwerp dichter dan 50 cm voor de sensor van de robot staat
        if(BP.get_sensor(PORT_2, Ultrasonic2) == 0 && Ultrasonic2.cm < 50){
            cout << "Grab object " << endl << Cm: << Ultrasonic2.cm;        //laat zien op het scherm dat er een object gevonden is

            if(BP.get_sensor(PORT_2, Ultrasonic2) == 0 && Ultrasonic2.cm < 8){
                stop();                                 //Als het voorwerp minder dan 8 cm voor de sensor staat stop de motoren
                BP.set_motor_power(PORT_D, 20);         //pak het voorwerp vast met de grijper
                break;
            }
            else{
                fwd(500);       //zolang het voorwerp minder dan 50cm voor het voorwerp staat en meer dan 8 cm rij naar voren
            }
        }else if(BP.get_sensor(PORT_2, Ultrasonic2) == 0 && Ultrasonic2.cm > 49 || Ultrasonic2.cm < 0){
            break;  //als het voorwerp verder dan 50cm van de sensor vandaan staat of als de sensor een waarde geeft kleiner
                    // dan 0 (wat alleen weergeven wordt als de afstand te groot is om te lezen of te klein) dan breakt het uit de loop

        }
        else{
            BP.set_motor_position(PORT_D, -4);      //zorgt er voor dat zodra er geen voorwerp meer zichtbaar is dat de klauwen weer open gaan
        }
    }
}

void control(sensor_ultrasonic_t Ultrasonic2){
    while(true){
        usleep(50000);

        if(BP.get_sensor(PORT_2, Ultrasonic2) == 0 && Ultrasonic2.cm < 17){
            stop();
            break;
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
    detect(Ultrasonic2);                //roept de functie aan


}

// Signal handler that will be called when Ctrl+C is pressed to stop the program
void exit_signal_handler(int signo){
    if(signo == SIGINT){
        BP.reset_all();    // Reset everything so there are no run-away motors
        exit(-2);
    }
}