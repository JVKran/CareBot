#include <opencv2/opencv.hpp>
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <fstream>
#include "BrickPi3.h"
#include <signal.h>
#include <iomanip>
#include <ctime>


using namespace cv;
using namespace std;

BrickPi3 BP;


int ct = 0; 
char tipka;
char filename[100]; // For filename
int  c = 1; // For filename

void exit_signal_handler(int signo);

Mat redFilter(const Mat& src){
    assert(src.type() == CV_8UC3);

    Mat redOnly;
    inRange(src, Scalar(0, 0, 0), Scalar(80, 80, 255), redOnly);

    return redOnly;
}

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
	BP.set_motor_limits(PORT_B, 100, 0);
	BP.set_motor_limits(PORT_C, 100, 0);
	BP.set_motor_limits(PORT_D, 100, 0);
}

void circle(int speed=50, int insideSpeed=2){
        BP.set_motor_dps(PORT_B, speed);
        BP.set_motor_dps(PORT_C, speed/insideSpeed);
}

void solidLeft(int speed=45){
	BP.set_motor_dps(PORT_B, speed*1.07);
        BP.set_motor_dps(PORT_C, -speed);
}

void solidRight(int speed=45){
	BP.set_motor_dps(PORT_B, -speed*1.07);
        BP.set_motor_dps(PORT_C, speed);
}

void manualDirection(int left=15, int right=15){
    BP.set_motor_dps(PORT_B, left*1.07);
    BP.set_motor_dps(PORT_C, right);
}

int main(int, char**){
    initialize();
    sensor_color_t      Color1;
    sensor_ultrasonic_t Ultrasonic2;
    sensor_light_t      Light3;
    sensor_touch_t      Touch4;
    fwd();
    Mat frame;
    string line;
    vector<string> y(3);
    int j = 0;
    string tmp;
    ifstream myfile ("/dev/rfcomm0");
    //--- INITIALIZE VIDEOCAPTURE
    VideoCapture cap;
    // open the default camera using default API
    cap.open(0);
    // check if we succeeded
    if (!cap.isOpened()) {
        cerr << "ERROR! Unable to open camera\n";
        return -1;
    }
    if (!myfile.is_open()) {
        cerr << "ERROR! Unable to open serial port\n";
        return -1;
    }
    
    //--- GRAB AND WRITE LOOP
    for (;;)
    {
	time_t current_time;
	current_time = time(NULL);
	if(current_time %5==0){
        getline (myfile,line);
        if(line.size() == 1){
          cout << "Ga nu exiten... DDDDDOOOOOOOOEEEEEEEIIIIIII!!!!!!!" << line << endl;
	  exit(stoi(line)); 
	}
        }
        // wait for a new frame from camera and store it into 'frame'
        cap.read(frame);

        if (frame.empty()) {
            cerr << "ERROR! blank frame grabbed\n";
            break;
        }


        usleep(500); // Sleep is mandatory - for no leg!
        Mat redOnly = redFilter(frame);


        // show live and wait for a key with timeout long enough to show images
        imshow("CAMERA 1", frame);  // Window name
        imshow("Red Camera", redOnly);  // Window name
	if(cv::countNonZero(redOnly) > 50000 || (BP.get_sensor(PORT_2, Ultrasonic2) == 0 && Ultrasonic2.cm < 15)){
		cv::Mat left = redOnly(cv::Range(0, redOnly.rows -1), cv::Range(0, redOnly.cols / 2 -1));
		cv::Mat right = redOnly(cv::Range(0, redOnly.rows -1), cv::Range(redOnly.cols / 2 + 1, redOnly.cols -1));
		int rightWhite = cv::countNonZero(right);
		int leftWhite = cv::countNonZero(left);
		if(leftWhite > rightWhite && Ultrasonic2.cm > 15){
			//cout << "Turn Right!" << endl;
			solidRight(200);
		} else if(Ultrasonic2.cm > 15) {
			//cout << "Turn Left!" << endl;
			solidLeft(200);
		} else {
			solidLeft(200);
			usleep(4000000);
		}
		usleep(100000);
	} else {
		fwd();
	}	
	tipka = cv::waitKey(30);
    }
    // the camera will be deinitialized automatically in VideoCapture destructor
    return 0;
}

void exit_signal_handler(int signo){
  if(signo == SIGINT){
    BP.reset_all();    // Reset everything so there are no run-away motors
    exit(-2);
  }
}
