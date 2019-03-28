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

void down(int speed=-45){
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
			
		string input;
		int IAmSpeed=90;
		while(mb.isRunning()) {
			input = mb.readMessage();  //blokkeert niet
			if(input != ""){
			cout << "input: " << input << endl;
			}
			if(input == "UP"){ 
				fwd(IAmSpeed);
			}else if(input == "LEFT"){
				left(500);
			}else if(input == "RIGHT"){
				right(500);
			}else if(input == "DOWN"){
				down(-IAmSpeed);
			}else if(input == "FIRE"){
				stop();
			}else if(input == "A"){
				cout<< "Normaal" << endl;
				IAmSpeed=90;
			}else if(input == "B"){
				cout<< "Snel" << endl;
				IAmSpeed=200;
			}else if(input == "C"){
				cout<< "I AM SPEED" << endl;
				IAmSpeed=300;
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
	
