#include <iostream>
#include <fstream>
#include <vector>
#include <signal.h>
#include "BrickPi3.h"

using namespace std;

BrickPi3 BP;

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

//bepaald per motor de kracht
void manualDirection(int left=15, int right=15){
    BP.set_motor_dps(PORT_B, left*1.07);                //1*07 in verband met ongelijkheid motoren
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

    string line;                        //hierin wordt de output van wat er in /dev/rfcomm0 staat opgeslagen
    vector<string> y(3);                //hierin worden de dingen die opgedeelt zijn van /dev/rfcomm0 opgeslagen voor verder gebruik
    int j = 0;                          //houd bij waar in de vector een waarde moet worden toegevoegd
    bool grab = false;                  //houd bij of de grijper open is of niet
    bool odd = true;                    //zorgt er voor dat de dubbele invoer bij /dev/rfcomm0 de grijper niet open en meteen weer dicht doet
    string tmp;                         //houd de verdeelde waardes tijdelijk vast
    ifstream myfile ("/dev/rfcomm0");                           //opent /dev/rfcomm0
    if (myfile.is_open()){                                      //checked of het geopend is
        while ( getline (myfile,line) ){                        //gaat door alle zinnen in /dev/rfcomm0
            tmp = "";                                           //zorgt er voor dat tmp weer leeg is
            j = 0;                                              //reset j zodat het opnieuw gebruikt kan worden
            for(unsigned int i = 0; i < line.size(); i++){      //loopt door alle karakters uit de string line
                if(line[i] != ','){                             //checked of het karakter niet gelijk is aan een komma zodat het toegevoegd kan worden aan tmp
                    tmp += line[i];                             //voegt het karakter toe
                }
                if(line[i] == ',' || line.size()-1 == i){       //checked of er een komma gevonden is of als het de laatste karakter is
                    y[j] = tmp;                                 //tmp wordt op de juiste locatie gezet in de vector
                    j++;                                        //j wordt een hoger zodat het volgende element in tmp op de juiste plek in de vector wordt toegevoegd
                    tmp = "";                                   //tmp wordt weer leeg gemaakt zodat het volgende element er in past
                }
            }
            cout << line << endl;
            //checked of /dev/rfcomm0 bepaalde waardes geeft die hoger of kleiner zijn dan een bepaalde waarde om te voorkomen dat de robot automatisch naar achteren rijd
            if(stoi(y[0]) > 2000 || stoi(y[0]) < 1600 || stoi(y[1]) > 2000 || stoi(y[1]) < 1600){
                //berekening om te bepalen hoeveel dps de motoren krijgen
                manualDirection((((stoi(y[0])-2000)/2)-((stoi(y[1])-2048)/4)), (((stoi(y[0])-2000)/2)+((stoi(y[1])-2048)/4)));
            } else {
                stop();
            }

            if(stoi(y[2])==0 && !grab && odd){              //checked of de grijper dicht is en zorgt er dan voor dat de grijper dicht gaat
                BP.set_motor_power(PORT_D, 0);              //stopt de motor
                BP.set_motor_position(PORT_D, -4);          //zorgt er voor dat de grijper opgen gaat
                grab = true;                                //verander de waarde van grab zodat de robot weet dat de grijper open is
                odd = false;                                //zorgt er voor dat de loop een keer niks doet met de grijper in verband met dubbele input voor grijper
                cout << "open\n";
            }else if(stoi(y[2])==0 && grab && odd){         //checked of de grijper open is en zorgt er dan voor dat de grijper dicht gaat
                BP.set_motor_power(PORT_D, 20);             //zet de motor power op 20 om er voor te zorgen dat de grijper grip blijft houden op het object
                grab = false;                               //verander de waarde van grab zodat de robot weet dat de grijper dicht is
                odd = false;                                //zorgt er voor dat de loop een keer niks doet met de grijper in verband met dubbele input voor grijper
                cout << "dicht\n";
            }else{
                odd = true;                                 //na een overgeslagen rondje in de loop zorgt dit er voor dat de grijper weer gebruikt kan worden
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
