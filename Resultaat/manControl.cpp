#include <iostream>
#include <fstream>
#include <vector>
#include <signal.h>
#include "BrickPi3.h"

using namespace std;

BrickPi3 BP;

void exit_signal_handler(int signo);

//Functie die er voor zorgt dat de motoren die bedoeld zijn om te rijden worden uitgezet
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

//Bepaald per motor de dps
void manualDirection(int left=15, int right=15){
    BP.set_motor_dps(PORT_B, left*1.07);		//1*07 in verband met ongelijkheid motoren
    BP.set_motor_dps(PORT_C, right);
}

int main () {
  signal(SIGINT, exit_signal_handler); // Register the exit function for Ctrl+C
  BP.detect(); // Make sure that the BrickPi3 is communicating and that the firmware is compatible with the drivers.
  int error;
  if(!initialize()){
	cout << "Initialisatie gelukt!";
  } else {
	cout << "Initialisatie mislukt...";
	BP.reset_all();
	exit(-2);
  }

  string line;					//Hierin wordt de output van wat er in /dev/rfcomm0 staat opgeslagen
  vector<string> y(3);				//Hierin worden de dingen die opgedeelt zijn van /dev/rfcomm0 opgeslagen voor verder gebruik
  int j = 0;					//Houd bij waar in de vector een waarde moet worden toegevoegd
  bool grab = false;				//Houd bij of de grijper open is of niet
  bool odd = true;				//Zorgt er voor dat de dubbele invoer bij /dev/rfcomm0 de grijper niet open en meteen weer dicht doet
  string tmp;					//Houd de verdeelde waardes tijdelijk vast
  
  ifstream myfile ("/dev/rfcomm0");				//Opent /dev/rfcomm0
  if (myfile.is_open()){					//Checked of het geopend is
    while ( getline (myfile,line) ){				//Gaat door alle zinnen in /dev/rfcomm0
      tmp = "";							//Zorgt er voor dat tmp weer leeg is
      j = 0;							//Reset j zodat het opnieuw gebruikt kan worden
      if(line.size() == 1){					
	  exit(stoi(line)); 					
      }
      for(unsigned int i = 0; i < line.size(); i++){		//Loopt door alle karakters uit de string line
	if(line[i] != ','){					//Checked of het karakter niet gelijk is aan een komma zodat het toegevoegd kan worden aan tmp
            tmp += line[i];					//Voegt het karakter toe
        }
        if(line[i] == ',' || line.size()-1 == i){		//Checked of er een komma gevonden is of als het de laatste karakter is
            y[j] = tmp;						//Tmp wordt op de juiste locatie gezet in de vector
	    j++;						//j wordt een hoger zodat het volgende element in tmp op de juiste plek in de vector wordt toegevoegd
            tmp = "";						//Tmp wordt weer leeg gemaakt zodat het volgende element er in past
       	}
      }
	    
      //Checked of /dev/rfcomm0 bepaalde waardes geeft die hoger of kleiner zijn dan een bepaalde waarde om te voorkomen dat de robot automatisch naar achteren rijd
      if(stoi(y[0]) > 2000 || stoi(y[0]) < 1600 || stoi(y[1]) > 2000 || stoi(y[1]) < 1600){
	//Berekening om te bepalen hoeveel dps de motoren krijgen
      	manualDirection((((stoi(y[0])-2000)/2)-((stoi(y[1])-2048)/4)), (((stoi(y[0])-2000)/2)+((stoi(y[1])-2048)/4)));
      } else {
	stop();						//Stopt de motoren
      }

      if(stoi(y[2])==0 && !grab && odd){		//Checked of de grijper dicht is en zorgt er dan voor dat de grijper dicht gaat
	BP.set_motor_power(PORT_D, 0);			//Stopt de motor		
	BP.set_motor_position(PORT_D, -4);		//Zorgt er voor dat de grijper opgen gaat
	grab = true;					//Verander de waarde van grab zodat de robot weet dat de grijper open is
	odd = false;					//Zorgt er voor dat de loop een keer niks doet met de grijper in verband met dubbele input voor grijper
	cout << "open\n";
      }else if(stoi(y[2])==0 && grab && odd){		//Checked of de grijper open is en zorgt er dan voor dat de grijper dicht gaat
	BP.set_motor_power(PORT_D, 20);			//Zet de motor power op 20 om er voor te zorgen dat de grijper grip blijft houden op het object
	grab = false;					//Verander de waarde van grab zodat de robot weet dat de grijper dicht is
        odd = false;					//Zorgt er voor dat de loop een keer niks doet met de grijper in verband met dubbele input voor grijper
	cout << "dicht\n";				//Zeg dat het dicht gaat
      }else{
 	odd = true;					//Na een overgeslagen rondje in de loop zorgt dit er voor dat de grijper weer gebruikt kan worden
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
