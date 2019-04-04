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
    BP.set_sensor_type(PORT_2, SENSOR_TYPE_NXT_ULTRASONIC);
    BP.set_motor_limits(PORT_D, 20, 0);
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

void detect(sensor_ultrasonic_t Ultrasonic2){
    int i = 0;

    BP.offset_motor_encoder(PORT_D, BP.get_motor_encoder(PORT_D));
    int32_t EncoderD = BP.get_motor_encoder(PORT_D);
    BP.set_motor_position(PORT_D, EncoderD);

    BP.set_motor_position(PORT_D, -4);		//De klauwen gaan open
    cout << "De klauwen worden nu geopend!" << endl;

    while(true){
        usleep(50000);

        if(BP.get_sensor(PORT_2, Ultrasonic2) == 0 && Ultrasonic2.cm < 50){
            cout << "Grab object " << i << endl;
            i++;
            if(BP.get_sensor(PORT_2, Ultrasonic2) == 0 && Ultrasonic2.cm < 8){
                stop();
                BP.set_motor_power(PORT_D, 20);
                break;
            }else{
                fwd(20);
            }
        }else{
            BP.set_motor_position(PORT_D, -4);
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
                cout<< "Detect" << endl;
                detect(Ultrasonic2);
            }else if(input == "B"){
                cout<< "Snel" << endl;
            }else if(input == "C"){
                cout<< "I AM SPEED" << endl;
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