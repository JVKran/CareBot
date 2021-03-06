#include "BluetoothSocket.h"
#include "BrickPi3.h" 		// for BrickPi3
#include <iostream>      	// for cout
#include <unistd.h>     	// for sleep
#include <signal.h>     	// for catching exit signals
#include <iomanip>		// for setw and setprecision

using namespace std;

BrickPi3 BP;

void exit_signal_handler(int signo);

void fwd(int speed=45){
    BP.set_motor_power(PORT_B, speed*1.07); //ivm ongelijkheid motoren.
    BP.set_motor_power(PORT_C, speed);
}

void stop(void){
	BP.set_motor_power(PORT_B, 0);
    	BP.set_motor_power(PORT_C, 0);
}

bool initialize(){
	if(BP.get_voltage_battery() < 10.85){
		cout << "Batterijspanning is te laag! Namelijk " << BP.get_voltage_battery() << "V. Script wordt getermineerd." << endl << endl;
		BP.reset_all();
		exit(-2);
	} else {
		cout << "Batterijspanning is goed. Namelijk " << BP.get_voltage_battery() << "V." << endl << endl;
	}
	BP.set_sensor_type(PORT_1, SENSOR_TYPE_NXT_COLOR_FULL);
	BP.set_sensor_type(PORT_2, SENSOR_TYPE_NXT_ULTRASONIC);
	BP.set_sensor_type(PORT_3, SENSOR_TYPE_NXT_LIGHT_ON);
	BP.set_sensor_type(PORT_4, SENSOR_TYPE_TOUCH);
	BP.set_sensor_type(PORT_2, SENSOR_TYPE_NXT_ULTRASONIC);
	BP.set_motor_limits(PORT_B, 100, 0);
	BP.set_motor_limits(PORT_C, 100, 0);
}

bool oldCalibrate(int & black, int & colorBlack, int & white, int & colorWhite, sensor_light_t Light3, sensor_color_t Color1){
	char input;
	int light = 0;
	int color = 0;
	cout << "\nPlaats op achtergrond en voer 'y' in.";
	cin >> input;
	if(input == 'y' ){
		for(unsigned int i = 0; i < 10; i++){
			BP.get_sensor(PORT_3, Light3);
			light+=Light3.reflected;
			BP.get_sensor(PORT_1, Color1);
			color+=Color1.reflected_blue;
			usleep(100000);
		}
	}
	// Doe meerdere lezingen en middel vervolgens om calibratie nauwkeuriger te maken.
	white = light / 10;
	colorWhite = color / 10;
	light = 0;
	color = 0;
	cout << "\nPlaats op lijn en voer 'y' in.";
	cin >> input;
	if(input == 'y'){
		for(unsigned int i = 0; i < 10; i++){
			BP.get_sensor(PORT_3, Light3);
			light += Light3.reflected;
			BP.get_sensor(PORT_1, Color1);
			color += Color1.reflected_blue;
			usleep(100000);
		}
	}
	black = light / 10;
	colorBlack = color / 10;
	cout << "\nGeef starstignaal (y)";
	cin >> input;
	if(input == 'y'){
		return true;
	} else {
		return false;
	}
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
void circle(int speed=50, int insideSpeed=2){
        BP.set_motor_dps(PORT_B, speed);
        BP.set_motor_dps(PORT_C, speed/insideSpeed);
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
	float kp = 1.5;
	float ki = 0;
	float kd = 0.001;
	float lasterror = 0;
	float value;
	float error;
	float integral = 0;
	float derivative;
	float correction;
	string input;
	BluetoothServerSocket serversock(2, 1);  //2 is het channel-number
	cout << "Bluetoothsocket gestart! Listening..." << endl;
	while(true){
		BluetoothSocket* clientsock = serversock.accept();
		cout << "Verbonden met: " << clientsock->getForeignAddress().getAddress() << endl;
		MessageBox& mb = clientsock->getMessageBox();
		cout << "Lijnvolger gestart!";
		while(mb.isRunning()){
			MessageBox& mb = clientsock->getMessageBox();
			input = mb.readMessage();
			if(input=="FIRE"){
				while(input!="UP"){
					input = mb.readMessage();
					if(input=="UP"){
						break;
					}
				}
			}
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
				usleep(600000);
				stop();
				if(Light3.reflected > midpoint){
					// Als zowel de kleurensensor als lichtsensor zwart aangeeft, moet er sprake zijn van een kruispunt
					input = mb.readMessage();
					while(input != "LEFT" && input != "RIGHT" && input != "UP"){
						input = mb.readMessage();
						usleep(500000);
					}
					cout << "Robot gaat: " << input << endl;
					if(input == "LEFT"){
						solidLeft(200);
					} else if (input == "RIGHT"){
						solidRight(200);
					}  else {
						continue;
					}
					usleep(1500000);	//Negeer de eerste keer dat de lijn voorbijkomt
					while(Light3.reflected<midpoint){
						// Blijf draaien totdat er weer een lijn gevonden wordt.
						BP.get_sensor(PORT_3, Light3);
					}
					if(input=="LEFT" || input =="RIGHT"){
						usleep(400000);		//Wacht nog 0,5 seconden om over de lijn heen te draaien
					}
					stop();
				} else {
					// Sprake van een scherpe bocht
					cout << "Scherpe bocht";
					BP.get_sensor(PORT_3, Light3);
					BP.get_sensor(PORT_1, Color1);
					if (Color1.reflected_blue < colorMidpoint){
						input = "LEFT";
						solidLeft(200);
					} else {
						solidRight(200);
					}
					while(Light3.reflected<midpoint){
						BP.get_sensor(PORT_3, Light3);
					}
					if(input=="LEFT"){
						usleep(500000);		//Wacht nog 0,5 seconden om over de lijn heen te draaien
					}
					stop();
				}
			}
			if(BP.get_sensor(PORT_2, Ultrasonic2) == 0 && Ultrasonic2.cm < 15){
				// Als de afstand kleiner is dan 10cm, draai dan naar rechts, maak een cirkel naar links
				// totdat de lijn wordt herkend. Rijd iets door zodat het op zijn plaats naar rechts draait. Hervat vervolgens.
				solidRight(200);
				usleep(2000000);
				circle(500);
				BP.get_sensor(PORT_3, Light3);
				while(Light3.reflected<midpoint){			// Misschien vergoper verkeerd om
					BP.get_sensor(PORT_3, Light3);
				}
				cout << "Lijn na obstakel gevonden\n";
				stop();
				BP.get_sensor(PORT_3, Light3);
				solidRight(200);
				while(Light3.reflected<midpoint){
					// Blijf draaien totdat er weer een lijn gevonden wordt.
					BP.get_sensor(PORT_3, Light3);
				}
				cout<<"turned";
				//usleep(1500000);
			}
		}
	}
  cout << "Bluetoothverbinding verloren..." << endl;
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
		cout << "Calibratie voltooid. ";
	} else {
		cout << "Calibratie mislukt. Programma wordt getermineerd";
		BP.reset_all();
		exit(-2);
	}
	stop();
	followPIDLine(white, colorWhite, colorBlack, black, Light3, Color1, Ultrasonic2);
}

// Signal handler that will be called when Ctrl+C is pressed to stop the program
void exit_signal_handler(int signo){
  if(signo == SIGINT){
    BP.reset_all();    // Reset everything so there are no run-away motors
    exit(-2);
  }
}
