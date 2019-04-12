#include "BrickPi3.h" 		// for BrickPi3
#include <iostream>      	// for cout
#include <unistd.h>     	// for sleep
#include <signal.h>     	// for catching exit signals
#include <iomanip>		    // for setw and setprecision

using namespace std;

BrickPi3 BP;

void exit_signal_handler(int signo);

bool initialize(){
    BP.set_sensor_type(PORT_1, SENSOR_TYPE_NXT_COLOR_FULL);
    BP.set_sensor_type(PORT_2, SENSOR_TYPE_NXT_ULTRASONIC);
    BP.set_sensor_type(PORT_3, SENSOR_TYPE_NXT_LIGHT_ON);
    BP.set_motor_limits(PORT_B, 60, 0);
    BP.set_motor_limits(PORT_C, 60, 0);
}

bool calibrate(int & black, int & white, sensor_light_t Light3){
    char input;                                                 //wordt gebruikt voor de gebruikers input
    cout << "Plaats op achtergrond en voer 'y' in.";                 
    cin >> input;                                               //wacht op invoer van de gebruiker
    
    //bepaald welke waarde de ondergrond heeft
    if(input == 'y'){                                           //checked of de invoer van de gebruiker gelijk is aan 'y'
        BP.get_sensor(PORT_3, Light3);                          //zet de sensor aan
        white = Light3.reflected;                               //leest de sensor uit en geeft de waarde aan de int white mee
    }
    cout << "Plaats op lijn en voer 'y' in.";
    cin >> input;                                               //vraagt weer om invoer van de gebruiker
    
    //bepaald welke waarde de lijn heeft
    if(input == 'y'){                                           //checked of de invoer van de gebruiker gelijk is aan 'y'
        BP.get_sensor(PORT_3, Light3);                          //zet de sensor aan
        black = Light3.reflected;                               //leest de sensor uit en geeft de waarde aan de int black mee
    }
    return true;                                                
}

//functie die er voor zorgt dat de robot stopt met rijden als dat nodig is
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

//functie om naar voren te rijden
void fwd(int speed=45){
    BP.set_motor_power(PORT_B, speed*1.07); //ivm ongelijkheid motoren.
    BP.set_motor_power(PORT_C, speed);
}

//functie om naar links te draaien met dps
void left(int speed=45){
    BP.set_motor_dps(PORT_B, speed*1.07);
    BP.set_motor_dps(PORT_C, -speed);
}

//functie om naar rechts te draaien met dps
void right(int speed=45){
    BP.set_motor_dps(PORT_B, -speed*1.07);
    BP.set_motor_dps(PORT_C, speed);
}

//elke motor kan met deze functie aparte snelheid krijgen
void manualDirection(int left=15, int right=15){
    BP.set_motor_dps(PORT_B, left*1.07);
    BP.set_motor_dps(PORT_C, right);
}

//formule voor het bepalen wanneer de robot moet bewegen/draaien
void followPIDLine(int white, int black, sensor_light_t Light3){
    int midpoint = ( white - black ) / 2 + black;                       //bepaald wat de waarde tussen wit en zwart is + zwart
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
        manualDirection(600+correction, 600-correction);                //bepaald de kracht van de motoren
        lasterror = error;
    }
}

int main(){
    signal(SIGINT, exit_signal_handler); // register the exit function for Ctrl+C
    BP.detect(); // Make sure that the BrickPi3 is communicating and that the firmware is compatible with the drivers.
    int error;
    
    //checked of de motoren/sensoren kloppen
    if(!initialize()){
        cout << "Initialisatie gelukt!";
    } else {
        cout << "Initialisatie mislukt...";
        return 1;
    }
    
    sensor_color_t      Color1;               
    sensor_ultrasonic_t Ultrasonic2;
    sensor_light_t      Light3;
    
    int white;                                  //de waarde van de ondergrond
    int black;                                  //de waarde van de lijn
    
    //checked of de calibratie gelukt is
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
