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
    BP.set_motor_limits(PORT_B, 100, 0);
    BP.set_motor_limits(PORT_C, 100, 0);
}

void stop(void){
    BP.set_motor_power(PORT_B, 0);
    BP.set_motor_power(PORT_C, 0);
}



void fwd(int speed=45){
    BP.set_motor_power(PORT_B, speed*1.07); //ivm ongelijkheid motoren.
    BP.set_motor_power(PORT_C, speed);
}

bool calibrate(int & black, int & colorBlack, int & white, int & colorWhite, sensor_light_t Light3, sensor_color_t Color1){
    char input;
    int light = 0;
    int color = 0;
    cout << "Calibratie voor witwaarden ";
    for(unsigned int i = 0; i < 10; i++){
        BP.get_sensor(PORT_3, Light3);
        light+=Light3.reflected;
        BP.get_sensor(PORT_1, Color1);
        color+=Color1.reflected_blue;
        usleep(100000);
    }
    white = light / 10;
    colorWhite = color / 10;
    cout << white << " en " << colorWhite << "."<< endl;
    fwd(25);
    light = 0;
    color = 0;
    while(Light3.reflected<white+400){
        // Blijf rijden totdat het donker is (er een lijn is).
        BP.get_sensor(PORT_3, Light3);
    }
    usleep(200000);
    stop();
    cout << "Calibratie voor zwartwaarden ";
    for(unsigned int i = 0; i < 10; i++){
        BP.get_sensor(PORT_3, Light3);
        light += Light3.reflected;
        BP.get_sensor(PORT_1, Color1);
        color += Color1.reflected_blue;
        usleep(100000);
    }
    black = light / 10;
    colorBlack = color / 10;
    cout << black << " en " << colorBlack << "." << endl;
    fwd(25);
    usleep(400000);
    if(abs(white - black) > 40){
        return true;
    } else {
        return false;
    }
}



void left(int speed=45){
    BP.set_motor_dps(PORT_B, speed*1.07);
    BP.set_motor_dps(PORT_C, -speed);
}

void solidLeft(int speed=45){
    BP.set_motor_dps(PORT_B, speed*1.07);
    BP.set_motor_dps(PORT_C, -speed);
}

void solidRight(int speed=45){
    BP.set_motor_dps(PORT_B, -speed*1.07);
    BP.set_motor_dps(PORT_C, speed);
}

void right(int speed=45){
    BP.set_motor_dps(PORT_B, -speed*1.07);
    BP.set_motor_dps(PORT_C, speed);
}

void manualDirection(int left=15, int right=15){
    BP.set_motor_dps(PORT_B, left*1.07);
    BP.set_motor_dps(PORT_C, right);
}

void followPIDLine(int white, int colorWhite, int colorBlack, int black, sensor_light_t Light3, sensor_color_t Color1, sensor_ultrasonic_t Ultrasonic2){
    int midpoint = ( white - black ) / 2 + black;			//Midpoint lichtsensor
    int colorMidpoint = ( colorWhite- colorBlack) / 2 + colorBlack;	//Midpoint kleurensensor
    float kp = 1.6;
    float ki = 0;
    float kd = 0.002;
    float lasterror = 0;
    float value;
    float error;
    float integral = 0;
    float derivative;
    float correction;
    char input;
    while(true){
        BP.get_sensor(PORT_3, Light3);
        BP.get_sensor(PORT_1, Color1);
        value = Light3.reflected;
        error = midpoint - value;
        integral = error + integral;
        derivative = error - lasterror;
        correction = kp * error + ki * integral + kd * derivative;
        manualDirection(600+correction, 600-correction);
        lasterror = error;
        // Als er een brede lijn is (kruispunt) of er heel veel gecorigeerd moet worden (einde lijn/scherpe bocht)
        cout << "Loopje"<<endl;
        if(Color1.reflected_blue < colorMidpoint || error > 280){
            BP.get_sensor(PORT_3, Light3);
            stop();
            fwd(40);
            // Rijd naar voren om op zijn plaats (x,y: 0,0) te draaien
            usleep(500000);
            stop();


        } else {
            // Sprake van een scherpe bocht
            cout << "Scherpe bocht";
            BP.get_sensor(PORT_3, Light3);
            BP.get_sensor(PORT_1, Color1);
            if (Color1.reflected_blue < colorMidpoint){
                input = 'l';
                solidLeft(200);
            } else {
                solidRight(200);
            }
            while(Light3.reflected<midpoint){
                BP.get_sensor(PORT_3, Light3);
            }
            if(input=='l'){
                usleep(500000);		//Wacht nog 0,5 seconden om over de lijn heen te draaien
            }
            stop();
        }
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
        BP.reset_all();
        exit(-2);
    }
    sensor_color_t      Color1;
    sensor_ultrasonic_t Ultrasonic2;
    sensor_light_t      Light3;
    sensor_touch_t      Touch4;
    int white;
    int black;
    int colorWhite;
    int colorBlack;
    if(calibrate(black, colorBlack, white, colorWhite, Light3, Color1)){
        cout << "Calibratie gelukt met " << black << " en " << colorBlack << " als zwartwaarden en " << white << " en " << colorWhite << " als witwaarden.\n";
    } else {
        cout << "Calibratie mislukt. Nog 1 poging.";
        if(calibrate(black, colorBlack, white, colorWhite, Light3, Color1)){
            cout << "Calibratie gelukt met " << black << " en " << colorBlack << " als zwartwaarden en " << white << " en " << colorWhite << " als witwaarden.\n";
        } else {
            cout << "Calibratie mislukt...";
            BP.reset_all();
            exit(-2);
        }
    }
    followPIDLine(white, colorWhite, colorBlack, black, Light3, Color1, Ultrasonic2);
}

// Signal handler that will be called when Ctrl+C is pressed to stop the program
void exit_signal_handler(int signo){
    if(signo == SIGINT){
        BP.reset_all();    // Reset everything so there are no run-away motors
        exit(-2);
    }
}
