#include "BluetoothSocket.h"
#include <unistd.h>
#include "BrickPi3.h" 		// for BrickPi3
#include <iostream>		
#include <signal.h>     	// for catching exit signals
#include <iomanip>		// for setw and setprecision

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
	BP.set_motor_limits(PORT_B, 0, 0);
	BP.set_motor_limits(PORT_C, 0, 0);
}

void fwd(int speed=45){
    BP.set_motor_dps(PORT_B, speed); //ivm ongelijkheid motoren.
    BP.set_motor_dps(PORT_C, speed);
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

void down(int speed=-45){
	BP.set_motor_dps(PORT_B, speed); //ivm ongelijkheid motoren.
    	BP.set_motor_dps(PORT_C, speed);
}

void detect(sensor_ultrasonic_t Ultrasonic2){
	BP.offset_motor_encoder(PORT_D, BP.get_motor_encoder(PORT_D));
	int32_t EncoderD = BP.get_motor_encoder(PORT_D);
	BP.set_motor_position(PORT_D, EncoderD);
	
	BP.set_motor_position(PORT_D, -4);		//De klauwen gaan open
	cout << "De klauwen worden nu geopend!" << endl;

	while(true){
		usleep(50000);      //slaap voor een korte tijd om betrouwbaardere waardes uit te lezen

        //checked of er een voorwerp dichter dan 50 cm voor de sensor van de robot staat
		if(BP.get_sensor(PORT_2, Ultrasonic2) == 0 && Ultrasonic2.cm < 50){
			cout << "Cm: " << Ultrasonic2.cm << endl;                           //laat zien op het scherm dat er een object gevonden is

			if(BP.get_sensor(PORT_2, Ultrasonic2) == 0 && Ultrasonic2.cm < 8){
				stop();                                 //Als het voorwerp minder dan 8 cm voor de sensor staat stop de motoren
				BP.set_motor_power(PORT_D, 20);         //pak het voorwerp vast met de grijper
				break;                                  //na het oppakken van het object stop de loop
			}
			else{
				fwd(500);           //zolang het voorwerp minder dan 50cm voor het voorwerp staat en meer dan 8 cm rij naar voren
			}
 		}else if(BP.get_sensor(PORT_2, Ultrasonic2) == 0 && Ultrasonic2.cm > 49 || Ultrasonic2.cm < 0){
			break;                  //als het voorwerp verder dan 50cm van de sensor vandaan staat of als de sensor een waarde geeft kleiner
                                    // dan 0 (wat alleen weergeven wordt als de afstand te groot is om te lezen of te klein) dan breakt het uit de loop
		}
		else{
			BP.set_motor_position(PORT_D, -4);  //zorgt er voor dat zodra er geen voorwerp meer zichtbaar is dat de klauwen weer open gaan
		}
	}
}

// functie die er voor zorgt dat de snelheid van het draaien en recht doorgaan kan worden verhoogt
// de variabele IamSpeed is bedoelt voor rechtdoor en de variabele bocht is bedoelt voor links en rechts
// aangezien bochten en vooruit en achteruit niet dezelfde waardes kunnen hebben in verband met de rupsbanden
void speed(int & IamSpeed,int & bocht){
    //een check die controleerd of de waardes groter zijn dan de orginele waardes of dat ze gelijk zijn aan de orginele waardes
	if(IamSpeed > 50 && bocht > 150){   //zodra de orginele waardes kleiner zijn dan de huidige waardes verander terug naar orginele waardes
		cout<< "Sloom" << endl;
		IamSpeed = 500;         //snelheid wordt aangepast naar 500 degrees per second
		bocht = 150;            //de bochtsnelheid wordt aangepast naar 150 degrees per second
	}else{      //in elk ander geval wordt het tempo verhoogd naar een vastgestelde waarde
		cout<< "I AM SPEED" << endl;
		IamSpeed = 1000;        //snelheid wordt aangepast naar 1000 degrees per second
		bocht = 500;            //de bochtsnelheid wordt aangepast naar 500 degrees per second
	}
}

//in deze functie wordt er gecontroleerd bij het rechtdoor rijden of er een object in de weg staat of niet
//en zodra er een object in de weg staat stopt de robot met naar voren rijden
void control(sensor_ultrasonic_t Ultrasonic2, BluetoothSocket* clientsock){
		string input=" ";   //variabele waarin de input van de gebruiker komt te staan

	while(true){
		usleep(50000);                                  //slaapt kort tussen door voor betrouwbaardere waardes
		cout << "Cm: " << Ultrasonic2.cm << endl;       //weergeeft de afstand
		MessageBox& mb = clientsock->getMessageBox();   //checked voor input
		input = mb.readMessage();                       //checked voor input
		if(input != " "){       //checked of de input niet gelijk is aan een spacie
			break;              //stopt de loop als dat zo is
		}
		//checked of er een voorwerp op 20 cm afstand is
		if(BP.get_sensor(PORT_2, Ultrasonic2) == 0 && Ultrasonic2.cm < 20){
			stop();             //stopt de motoren
			break;              //stopt de loop
		}
	}
}


int main() {
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
	
	BluetoothServerSocket serversock(2, 1);  //2 is het channel-number
	cout << "listening" << endl;
	while(true) {
		BluetoothSocket* clientsock = serversock.accept();
		cout << "accepted from " << clientsock->getForeignAddress().getAddress() << endl;
		MessageBox& mb = clientsock->getMessageBox();
			
		string input;
		int IAmSpeed=500;               //standaard degrees per second voor vooruit en achteruit
		int bocht = 150;                //standaard degrees per second voor links en rechts
		while(mb.isRunning()) {
			input = mb.readMessage();  //blokkeert niet
			if(input != ""){
			cout <<endl<<"input: " << input << endl;
			}
			if(input == "UP"){ 
				fwd(IAmSpeed);
				control(Ultrasonic2,clientsock);
			}else if(input == "LEFT"){
				left(bocht);
			}else if(input == "RIGHT"){
				right(bocht);
			}else if(input == "DOWN"){
				down(-IAmSpeed);
			}else if(input == "FIRE"){
				stop();
			}else if(input == "A"){
				cout<< "Detect" << endl;
				detect(Ultrasonic2);
			}else if(input == "B"){
				cout<< "Open" << endl;
				BP.set_motor_position(PORT_D, -4);
			}else if(input == "C"){
				
				speed(IAmSpeed, bocht);
			}
			//doe andere dingen.
			cout << ".";
			cout.flush();
			usleep(500000); // wacht 500 ms
		}	
		
		clientsock->close();
			
	}
}

// Signal handler that will be called when Ctrl+C is pressed to stop the program
void exit_signal_handler(int signo){
  if(signo == SIGINT){
    BP.reset_all();    // Reset everything so there are no run-away motors
    exit(-2);
  }
}
	
