#include <iostream>
#include <opencv2/opencv.hpp>
#include <unistd.h>
#include <fstream>
#include <stdlib.h>
#include <vector>
#include <signal.h>
#include "BrickPi3.h"
#include <iomanip>

using namespace std;
using namespace cv;

BrickPi3 BP;

int ct = 0; 
char tipka;
char filename[100]; // For filename
int  c = 1; // For filename

void exit_signal_handler(int signo);

Mat redFilter(const Mat& src){
    // Geef alleen de pixels weer waarvan de waarden tussen 0,0,0 en 125,125,255 (RGB) liggen
    assert(src.type() == CV_8UC3);
    Mat redOnly;
    inRange(src, Scalar(0, 0, 0), Scalar(125, 125, 255), redOnly);
    return redOnly;
}

bool initialize(){
	if(BP.get_voltage_battery() < 10.85){
		cout << "Batterijspanning is te laag! Namelijk " << BP.get_voltage_battery() << "V. Script wordt getermineerd." << endl << endl;
		BP.reset_all();
		exit(-2);
	} else {
		cout << "Batterijspanning is goed. Namelijk " << BP.get_voltage_battery() << "V." << endl << endl;
	}
	BP.set_sensor_type(PORT_2, SENSOR_TYPE_NXT_ULTRASONIC);
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
  sensor_ultrasonic_t Ultrasonic2;
  string line;
  vector<string> y(3);
  int j = 0;
  string tmp;
  Mat frame;
  VideoCapture cap;
  cap.open(0);
  char tipka;
  bool grab = false;
	
  if (!cap.isOpened()) {
        cerr << "Fout tijdens het openen van de Camera!\n";
        return -1;
  }
	
  ifstream myfile ("/dev/rfcomm0"); //Virtuele seriële poort
  int fd = open("/dev/rfcomm0", O_WRONLY | O_APPEND);
	
  if (myfile.is_open() && fd >= 0){
    while ( getline (myfile,line) ){
      cap.read(frame);
      if (frame.empty()) {
            cerr << "Leeg frame!\n";
            break;
      }
      usleep(500);
	    
      Mat redOnly = redFilter(frame);
	    
      imshow("CAMERA 1", frame);  // Window name
      imshow("Blank Camera", redOnly);  // Window name
	    
      tmp = "";
      j = 0;
	    
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
      // Als er meer dan 50000 witte pixels zijn
      if(cv::countNonZero(redOnly) > 50000){
		cv::Mat left = redOnly(cv::Range(0, redOnly.rows -1), cv::Range(0, redOnly.cols / 2 -1)); 			//Het beeld verdelen naar links
		cv::Mat right = redOnly(cv::Range(0, redOnly.rows -1), cv::Range(redOnly.cols / 2 + 1, redOnly.cols -1));	//Het beeld verdelen naar rechts
		int rightWhite = cv::countNonZero(right);		//Het tellen van witte waardes aan de rechter kant
		int leftWhite = cv::countNonZero(left);			//Het tellen van witte waardes aan de linker kant
	        // Als er links meer witte pixels zijn dan rechts geef dan rode balk weer op scherm (links)
		if(leftWhite > rightWhite){	
			write(fd,"leftDanger#",50);		//Rode balk links
		} else {
			write(fd,"rightDanger#",50);		//Rode balk rechts
		}
      } else {
	    write(fd,"noDirectionDanger#",50);			//Geen balk links of rechts
      }
      tipka = cv::waitKey(30);
      // Als de afstand voor kleiner is dan 25cm, geef dan een rode balk boven het scherm weer
      if(BP.get_sensor(PORT_2, Ultrasonic2) == 0 && Ultrasonic2.cm < 25){
	      write(fd,"frontDanger#",50);
      } else {
	       // Haal de rode balk anders weg
	       write(fd,"noFrontDanger#",50);
      }
      cout << line << endl;
      if(stoi(y[0]) > 2000 || stoi(y[0]) < 1600 || stoi(y[1]) > 2000 || stoi(y[1]) < 1600){				//Voorwaarden om het nog op zijn eigen plek te laten rond draaien
      	manualDirection((((stoi(y[0])-2000)/2)-((stoi(y[1])-2048)/4)), (((stoi(y[0])-2000)/2)+((stoi(y[1])-2048)/4))); //Formule om het te laten bewegen via joystick
      }
      if(stoi(y[2])==0){					//Als er op de knop wordt gedrukt, krijg je de waarde 0 en gaat het grijpen, stoi maakt het een getal
		BP.set_motor_power(PORT_D, 0);			//De power stoppen
		BP.set_motor_position(PORT_D, -4);		//De klauwen open maken
		grab = true;
      }
      if(BP.get_sensor(PORT_2, Ultrasonic2) == 0 && Ultrasonic2.cm < 8 && grab){		//Als de Ultrasone een afstand heeft lager dan 8 en grab is true, voer het uit
	BP.set_motor_power(PORT_D, 20);				//De klauwen grijpen vast
	grab = false;
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
