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
	BP.set_motor_limits(PORT_B, 60, 0);
	BP.set_motor_limits(PORT_C, 60, 0);
}

void fwd(int speed=45){				//functie om naar voren te rijden
    BP.set_motor_power(PORT_B, speed*1.07); //ivm ongelijkheid motoren.
    BP.set_motor_power(PORT_C, speed);
}

void stop(void){				//functie om de motoren uit te zetten
	BP.set_motor_power(PORT_B, 0);
    	BP.set_motor_power(PORT_C, 0);
}

void left(int speed=45){			//functie om naar links te draaien
    BP.set_motor_dps(PORT_B, speed*1.07);
    BP.set_motor_dps(PORT_C, -speed);
}

void right(int speed=45){			//functie om naar rechts te draaien
    BP.set_motor_dps(PORT_B, -speed*1.07);
    BP.set_motor_dps(PORT_C, speed);
}

void down(int speed=-45){			//functie om naar achteren te gaan
	BP.set_motor_power(PORT_B, speed*1.07); //ivm ongelijkheid motoren.
    	BP.set_motor_power(PORT_C, speed);
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
	
	BluetoothServerSocket serversock(2, 1);  //2 is het channel-number
	cout << "listening" << endl;
	while(true) {
		BluetoothSocket* clientsock = serversock.accept();
		cout << "accepted from " << clientsock->getForeignAddress().getAddress() << endl;
		MessageBox& mb = clientsock->getMessageBox();
			
		string input;			//hierin staat de input die wordt gegeven door een verbonden mobiel.
		int IAmSpeed=90;		//hierin staat de snelheid voor de motoren naar voren en naar achteren
		while(mb.isRunning()) {
			input = mb.readMessage();  		//blokkeert niet
			if(input != ""){			//controlleerd of de input niet gelijk is aan niks
			cout << "input: " << input << endl;	//laat op het scherm zien wat de input is
			}
			if(input == "UP"){ 
				fwd(IAmSpeed);			//Als de input pijltje omhoog is gaat het naar voren rijden
			}else if(input == "LEFT"){
				left(500);			//Als de input pijltje naar links is gaat het naar links draaien
			}else if(input == "RIGHT"){
				right(500);			//Als de input pijltje naar rechts is gaat het naar rechts draaien
			}else if(input == "DOWN"){
				down(-IAmSpeed);		//Als de input pijltje omlaag is gaat het naar achteren rijden
			}else if(input == "FIRE"){
				stop();				//Als je op de middelste knop drukt stopt het met wat het aan het doen is
			}else if(input == "A"){			//Werkt nog niet volledig
				cout<< "Normaal" << endl;
				IAmSpeed=90;			//Veranderd de snelheid naar 90, als je op knop A drukt
			}else if(input == "B"){			//Werkt nog niet volledig!
				cout<< "Snel" << endl;
				IAmSpeed=200;			//Veranderd de snelheid naar 200, als je op knop A drukt
			}else if(input == "C"){			//Werkt nog niet volledig!
				cout<< "I AM SPEED" << endl;
				IAmSpeed=300;			//Veranderd de snelheid naar 300, als je op knop A drukt
			}
			//doe andere dingen.
			cout << ".";
			cout.flush();
			usleep(500000); // wacht 500 ms voor volgende input om overgevoeligheid in het programma te voorkomen
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
	
