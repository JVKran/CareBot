#include "BrickPi3.h" 		// for BrickPi3
#include <iostream>      	// for cout
#include <unistd.h>     	// for sleep
#include <signal.h>     	// for catching exit signals
#include <iomanip>		// for setw and setprecision

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

bool calibrate(int & black, int & white, sensor_light_t Light3){
	char input;
	cout << "Plaats op achtergrond en voer 'y' in.";
	cin >> input;
	if(input == 'y'){
		BP.get_sensor(PORT_3, Light3);
		white = Light3.reflected;
	}
	cout << "Plaats op lijn en voer 'y' in.";
	cin >> input;
	if(input == 'y'){
		BP.get_sensor(PORT_3, Light3);
		black = Light3.reflected;
	}
	return true;
}

void stop(void){
	BP.set_motor_power(PORT_B, 0);
    	BP.set_motor_power(PORT_C, 0);
}

// De robot rijdt in een vierkant. secs bepaalt hoelang de robot rechtdoor rijdt
void square(int secs=2){
    for(unsigned int i=0;i<4;i++){ // Rij vooruit
        BP.set_motor_power(PORT_B, 60);
        BP.set_motor_power(PORT_C, 56);
        sleep(secs);
        // ga naar rechts
        BP.set_motor_dps(PORT_B, -45);
        BP.set_motor_dps(PORT_C, 45);
        usleep(700000);
        stop();
    }
}
// De robot rijdt in een cirkel. Hoe kleiner de insideSpeed, hoe groter de radius.
void circle(int duration=5, int speed=50, int insideSpeed=2){
    for(unsigned int i=0;i<4;i++){
        BP.set_motor_power(PORT_B, speed);
        BP.set_motor_power(PORT_C, speed/insideSpeed);
        sleep(duration);
        stop();
    }
}

void fwd(int speed=45){
    BP.set_motor_power(PORT_B, speed*1.07); //ivm ongelijkheid motoren.
    BP.set_motor_power(PORT_C, speed);
}

void left(int speed=45){
    BP.set_motor_dps(PORT_B, speed*1.07);
    BP.set_motor_dps(PORT_C, -speed);
}

void right(int speed=45){
    BP.set_motor_dps(PORT_B, -speed*1.07);
    BP.set_motor_dps(PORT_C, speed);
}

void manualDirection(int left=15, int right=15){
    BP.set_motor_dps(PORT_B, left*1.07);
    BP.set_motor_dps(PORT_C, right);
}

void followPLine(int white, int black, sensor_light_t Light3){
	int midpoint = ( white - black ) / 2 + black;
	float kp;
	cout << "Geef een KP: ";
	cin >> kp;
	int value;
	float correction;
	while(true){
		BP.get_sensor(PORT_3, Light3);
		value = Light3.reflected;
		correction = kp * ( midpoint - value );
		manualDirection(200+correction, 200-correction);
	}
}

void followPIDLine(int white, int black, sensor_light_t Light3){
	int midpoint = ( white - black ) / 2 + black;
	float kp = 0.8;
	float ki = 0;
	float kd = 0.002;
	float lasterror = 0;
	float value;
	float error;
	float integral = 0;
	float derivative;
	float correction;
	while(true){
		BP.get_sensor(PORT_3, Light3);
		value = Light3.reflected;
		error = midpoint - value;
		integral = error + integral;
		derivative = error - lasterror;
		correction = kp * error + ki * integral + kd * derivative;
		manualDirection(600+correction, 600-correction);
		lasterror = error;
	}
}

int main(){
	signal(SIGINT, exit_signal_handler); // register the exit function for Ctrl+C
	BP.detect(); // Make sure that the BrickPi3 is communicating and that the firmware is compatible with the drivers.
	int error;
	if(!initialize()){
		cout << "Initialisatie gelukt!";
	} else {
		cout << "Initialisatie mislukt...";
		return 1;
	}
	sensor_color_t      Color1;
	sensor_ultrasonic_t Ultrasonic2;
	sensor_light_t      Light3;
	sensor_touch_t      Touch4;
	int white;
	int black;
	if(calibrate(black, white, Light3)){
		cout << "Calibratie gelukt met " << black << " als zwartwaarde en " << white << " als witwaarde";
	} else {
		cout << "Calibratie mislukt. Nog 1 poging.";
		if(calibrate(black, white, Light3)){
			cout << "Calibratie gelukt met " << black << " als zwartwaarde en " << white << " als witwaarde";
		} else {
			cout << "Calibratie mislukt...";
			return 1;
		}
	}
	followPIDLine(white, black, Light3);
}

// Signal handler that will be called when Ctrl+C is pressed to stop the program
void exit_signal_handler(int signo){
  if(signo == SIGINT){
    BP.reset_all();    // Reset everything so there are no run-away motors
    exit(-2);
  }
}
