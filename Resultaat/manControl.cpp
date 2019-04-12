#include <iostream>
#include <fstream>
#include <vector>
#include <signal.h>
#include "BrickPi3.h"

using namespace std;

BrickPi3 BP;

bool grab = false;
void exit_signal_handler(int signo);

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

	BP.set_motor_limits(PORT_B, 100, 0);
	BP.set_motor_limits(PORT_C, 100, 0);
}

void manualDirection(int left=15, int right=15){
    BP.set_motor_dps(PORT_B, left*1.07);
    BP.set_motor_dps(PORT_C, right);
}

int main () {
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

  string line;
  vector<string> y(3);
  int j = 0;
  bool grab = false;
  bool odd = true;
  string tmp;
  ifstream myfile ("/dev/rfcomm0");
  if (myfile.is_open()){
    while ( getline (myfile,line) ){
      tmp = "";
      j = 0;
      if(line.size() == 1){
	  exit(stoi(line)); 
      }
      for(unsigned int i = 0; i < line.size(); i++){
	if(line[i] != ','){
            tmp += line[i];
        }
        if(line[i] == ',' || line.size()-1 == i){
            y[j] = tmp;
	    j++;
            tmp = "";
       	}
      }
      if(stoi(y[0]) > 2000 || stoi(y[0]) < 1600 || stoi(y[1]) > 2000 || stoi(y[1]) < 1600){
      	manualDirection((((stoi(y[0])-2000)/2)-((stoi(y[1])-2048)/4)), (((stoi(y[0])-2000)/2)+((stoi(y[1])-2048)/4)));
      } else {
	stop();
      }

      if(stoi(y[2])==0 && !grab && odd){
	//De button geeft twee keer 0 mee met 1 keer drukken
	BP.set_motor_power(PORT_D, 0);
	BP.set_motor_position(PORT_D, -4);
	grab = true;
	odd = false;
	cout << "open\n";
      }else if(stoi(y[2])==0 && grab && odd){
	
	BP.set_motor_power(PORT_D, 20);
	grab = false;
        odd = false;
	cout << "dicht\n";
      }else{
 	odd = true;
    }
    }
  } else {
      cout << "Unable to open file"; 
  }
  return 0;
}

void exit_signal_handler(int signo){
  if(signo == SIGINT){
    BP.reset_all();    // Reset everything so there are no run-away motors
    exit(-2);
  }
}
