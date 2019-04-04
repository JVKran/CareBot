#include "BrickPi3.h" 		// for BrickPi3
#include <iostream>      	// for cout
#include <unistd.h>     	// for sleep
#include <signal.h>     	// for catching exit signals
#include <iomanip>		// for setw and setprecision
#include <vector>
#include <bitset>

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
	BP.set_motor_limits(PORT_B, 100, 0);
	BP.set_motor_limits(PORT_C, 100, 0);
	BP.set_motor_limits(PORT_D, 100, 0);
}

void grab(sensor_ultrasonic_t Ultrasonic2){
	BP.offset_motor_encoder(PORT_D, BP.get_motor_encoder(PORT_D));
	int32_t EncoderD = BP.get_motor_encoder(PORT_D);
	BP.set_motor_position(PORT_D, EncoderD);
	cout << "Motor gereset" << endl;
	string input;
	int IAmSpeed = 15;
	float Time = 1900000.0;
	uint8_t StateD;
	int8_t PowerD;
	int32_t PositionD;
	int16_t DPSD;
	while(true){
		cout << "g or o the claws?(Type m to modify speed and time. Type Stop to stop)" << endl;
		cin >> input;
		if(input == "g"){			
   			BP.set_motor_position(PORT_D, 4); 		//De klauwen gaan grijpen
			cout << "De klauwen gaan nu grijpen!" << endl;
			BP.get_motor_status(PORT_D, StateD, PowerD, PositionD, DPSD);
			cout << DPSD << endl;

			while(true){
				if(PowerD==8 && DPSD==0 && PositionD==0){
					break;
				}
				BP.get_motor_status(PORT_D, StateD, PowerD, PositionD, DPSD);
				printf("Encoder D: state: %d, power: %d, position: %d DPS: %d \n", StateD, PowerD, PositionD, DPSD);
				usleep(200000);
				}
			BP.set_motor_position(PORT_D, 0);
		}
		else if(input == "c"){
			BP.set_motor_position(PORT_D, 3);
		}
		else if(input == "o"){
			BP.set_motor_position(PORT_D, -5);		//De klauwen gaan open
			cout << "De klauwen worden nu geopend!" << endl;
		}
		else if(input == "Stop"){
			break;
		}
		else if(input == "m"){
			cout << "Snelheid veranderen: " << endl;
			cin >> IAmSpeed;
			cout << "Tijd veranderen(getal x 1 miljoen): " << endl;
			cin >> Time;
			Time *=1000000;
			cout << "De nieuwe tijd is: " << Time << endl;
		}
		
	}
}

int main(){
	signal(SIGINT, exit_signal_handler); // register the exit function for Ctrl+C
	BP.detect(); // Make sure that the BrickPi3 is communicating and that the firmware is compatible with the drivers.
	int error;

	if(!initialize()){
		cout << "Initialisatie gelukt!" << endl;
	} else {
		cout << "Initialisatie mislukt..." << endl;
		BP.reset_all();
		exit(-2);
	}

	sensor_ultrasonic_t Ultrasonic2;
	grab(Ultrasonic2);
}

// Signal handler that will be called when Ctrl+C is pressed to stop the program
void exit_signal_handler(int signo){
  if(signo == SIGINT){
    BP.reset_all();    // Reset everything so there are no run-away motors
    exit(-2);
  }
}
