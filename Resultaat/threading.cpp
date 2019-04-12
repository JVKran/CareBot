#include <iostream>
#include <unistd.h>
#include <fstream>
#include <vector>
#include <signal.h>
#include "BrickPi3.h"
#include <iomanip>
#include <thread>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

BrickPi3 BP;

bool grab = false;
void exit_signal_handler(int signo);

Mat redFilter(const Mat& src){			//Filtert afbeelding
    assert(src.type() == CV_8UC3);

    Mat redOnly;
    inRange(src, Scalar(0, 0, 0), Scalar(125, 125, 255), redOnly);		//Maakt een kleurfilten in de gespecificeerde limieten voor rgb en maakt een nieuwe matrix met dit filter

    return redOnly;
}

void readCam(sensor_ultrasonic_t Ultrasonic2){
  char tipka;				//Tipka is "key" in het fins
  Mat frame;
  VideoCapture cap;			//Maakt foto
  cap.open(0);				//Open de camera
  if (!cap.isOpened()) {		//Als de camera niet open kan, voer de code uit
        cerr << "ERROR! Unable to open camera\n";
  }
  while(true){
  cap.read(frame);
        if (frame.empty()) {
            cerr << "ERROR! blank frame grabbed\n";
            break;
        }
        usleep(500); // Sleep is mandatory - for no leg!
        Mat redOnly = redFilter(frame);
        // Show live and wait for a key with timeout long enough to show images
        imshow("CAMERA 1", frame);  // Window name
        imshow("Red Camera", redOnly);  // Window name
        if(cv::countNonZero(redOnly) > 50000 || (BP.get_sensor(PORT_2, Ultrasonic2) == 0 && Ultrasonic2.cm < 25)){			//als er na het filter veel witte pixels zijn 
		cv::Mat left = redOnly(cv::Range(0, redOnly.rows -1), cv::Range(0, redOnly.cols / 2 -1));				//verdeelt afbeelding in links en rechts
		cv::Mat right = redOnly(cv::Range(0, redOnly.rows -1), cv::Range(redOnly.cols / 2 + 1, redOnly.cols -1));	
		int rightWhite = cv::countNonZero(right);
		int leftWhite = cv::countNonZero(left);
		if(leftWhite > rightWhite){
			cout << "Turn Right!" << endl;				//Gaat de kant op met minste witte pixels
		} else {
			cout << "Turn Left!" << endl;
		}
	} else {
		cout << "Rechtdoor" << endl;					//Bij weinig witte pixels rechts
	}	
	tipka = cv::waitKey(30);
    }
    //The camera will be deinitialized automatically in VideoCapture destructor
}

void fwd(int speed=45){
    BP.set_motor_dps(PORT_B, speed*1.07); //Ivm ongelijkheid motoren.
    BP.set_motor_dps(PORT_C, speed);
}

void fwdDPS(int speed=45){
    BP.set_motor_dps(PORT_B, speed); //Ivm ongelijkheid motoren.
    BP.set_motor_dps(PORT_C, speed);
}

void stop(void){
	BP.set_motor_power(PORT_B, 0);
    	BP.set_motor_power(PORT_C, 0);
}
void detect(sensor_ultrasonic_t Ultrasonic2){
	BP.set_motor_position(PORT_D, -4);		//De klauwen gaan open
	cout << "De klauwen worden nu geopend!" << endl;

	while(true){
		usleep(50000);

		if(BP.get_sensor(PORT_2, Ultrasonic2) == 0 && Ultrasonic2.cm < 50){
			cout << "Cm: " << Ultrasonic2.cm << endl;
			
			if(BP.get_sensor(PORT_2, Ultrasonic2) == 0 && Ultrasonic2.cm < 8){
				stop();
				BP.set_motor_power(PORT_D, 20);
				break;
			}
			
 		}else if(BP.get_sensor(PORT_2, Ultrasonic2) == 0 && Ultrasonic2.cm > 49 || Ultrasonic2.cm < 0){
			break;
		}
		else{
			BP.set_motor_position(PORT_D, -4);
		}
	}
}


bool initialize(){
	if(BP.get_voltage_battery() < 10.85){		//Onderspanningsbeveiliging
		cout << "Batterijspanning is te laag! Namelijk " << BP.get_voltage_battery() << "V. Script wordt getermineerd." << endl << endl;
		BP.reset_all();
		exit(-2);
	} else {
		cout << "Batterijspanning is goed. Namelijk " << BP.get_voltage_battery() << "V." << endl << endl;
	}
	BP.set_sensor_type(PORT_1, SENSOR_TYPE_NXT_COLOR_FULL);			//Sensoren en motoren initialiseren
	BP.set_sensor_type(PORT_2, SENSOR_TYPE_NXT_ULTRASONIC);
	BP.set_sensor_type(PORT_3, SENSOR_TYPE_NXT_LIGHT_ON);
	BP.set_sensor_type(PORT_4, SENSOR_TYPE_TOUCH);
	BP.set_sensor_type(PORT_2, SENSOR_TYPE_NXT_ULTRASONIC);
	BP.set_motor_limits(PORT_B, 100, 0);					//Powerlimieten instellen
	BP.set_motor_limits(PORT_C, 100, 0);
}


void left(int speed=45){			//Bewegingsfuncities
    BP.set_motor_dps(PORT_B, speed*1.07);
    BP.set_motor_dps(PORT_C, -speed);
}

void right(int speed=45){
    BP.set_motor_dps(PORT_B, -speed*1.07);
    BP.set_motor_dps(PORT_C, speed);
}

void manualDirection(int left=15, int right=15){
    BP.set_motor_dps(PORT_B, left*1.07);
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
  sensor_color_t      Color1;			//Sensors toewijzen
  sensor_ultrasonic_t Ultrasonic2;
  sensor_light_t      Light3;
  sensor_touch_t      Touch4;
  string line;
  vector<string> y(3);
  int j = 0;
  bool grab = false;
  thread camera(readCam,Ultrasonic2);		//Verschillende threads maken
  string tmp;
  ifstream myfile ("/dev/rfcomm0");		//Via het virtuele bestand /dev/rfcomm0 met bluetooth communiceren
  if (myfile.is_open()){
    while ( getline (myfile,line) ){
      tmp = "";
      j = 0;			//Positie data in vector
      for(unsigned int i = 0; i < line.size(); i++){	//Data uitlezen van bluetooth voor x y as en knop
	if(line[i] != ','){
            tmp += line[i];
        }
        if(line[i] == ',' || line.size()-1 == i){
            y[j] = tmp;
	    j++;
            tmp = "";
       	}
      }
      cout << line << endl;
      if(stoi(y[0]) > 2000 || stoi(y[0]) < 1600 || stoi(y[1]) > 2000 || stoi(y[1]) < 1600){//Definieert deadzones voor inacurate centrering joystick
      	manualDirection((((stoi(y[0])-2000)/2)-((stoi(y[1])-2048)/4)), (((stoi(y[0])-2000)/2)+((stoi(y[1])-2048)/4)));
      } else {
	stop();
      }
      if(stoi(y[2])==0){								//Grijpen sls knop is ingedrukt
		BP.set_motor_power(PORT_D, 0);	
		BP.set_motor_position(PORT_D, -4);
		grab = true;
      }
      if(BP.get_sensor(PORT_2, Ultrasonic2) == 0 && Ultrasonic2.cm < 8 && grab){		//Als een object vast is, loslaten
	BP.set_motor_power(PORT_D, 20);
	
	grab = false;
      }
    }
  } else {
      cout << "Unable to open file"; 
  }
  camera.join();  //Join threads
  return 0;
}

void exit_signal_handler(int signo){
  if(signo == SIGINT){
    BP.reset_all();    // Reset everything so there are no run-away motors
    exit(-2);
  }
}
