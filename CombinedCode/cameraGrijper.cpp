#include <opencv2/opencv.hpp>
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include "BrickPi3.h"
#include <signal.h>
#include <iomanip>
#include "BluetoothSocket.h"


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
void fwdDPS(int speed=45){
    BP.set_motor_dps(PORT_B, speed); //ivm ongelijkheid motoren.
    BP.set_motor_dps(PORT_C, speed);
}
void down(int speed=-45){
	BP.set_motor_dps(PORT_B, speed); //ivm ongelijkheid motoren.
    	BP.set_motor_dps(PORT_C, speed);
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
			else{
				fwd(500);
			}
 		}else if(BP.get_sensor(PORT_2, Ultrasonic2) == 0 && Ultrasonic2.cm > 49 || Ultrasonic2.cm < 0){
			break;
		}
		else{
			BP.set_motor_position(PORT_D, -4);
		}
	}
}

void moreSpeed(int & IamSpeed,int & bocht){
	if(IamSpeed > 50 && bocht > 150){
		cout<< "Sloom" << endl;
		IamSpeed = 500;
		bocht = 150;
	}else{
		cout<< "I AM SPEED" << endl;
		IamSpeed = 1000;
		bocht = 500;
	}
}

void control(sensor_ultrasonic_t Ultrasonic2, BluetoothSocket* clientsock){
		string input;
	while(true){
		usleep(50000);
		cout << "Cm: " << Ultrasonic2.cm << endl;
		MessageBox& mb = clientsock->getMessageBox();
		input = mb.readMessage();
		if(input != ""){
			break;
		}
		if(BP.get_sensor(PORT_2, Ultrasonic2) == 0 && Ultrasonic2.cm < 20){
			
			stop();
			break;
		}
	}
}

int main(int, char**){
    initialize();
    sensor_color_t      Color1;
    sensor_ultrasonic_t Ultrasonic2;
    sensor_light_t      Light3;
    sensor_touch_t      Touch4;
    string input;
    int IAmSpeed=500;
    int bocht = 150;
    
    Mat frame;
    //--- INITIALIZE VIDEOCAPTURE
    VideoCapture cap;
    // open the default camera using default API
    cap.open(0);
    // check if we succeeded
    if (!cap.isOpened()) {
        cerr << "ERROR! Unable to open camera\n";
        return -1;
    }
    //--- GRAB AND WRITE LOOP
    cout << "Start grabbing" << endl
        << "Press a to terminate" << endl;
    for (;;)
    {
	BluetoothServerSocket serversock(2, 1);  //2 is het channel-number
	cout << "listening" << endl;
	while(true) {
		BluetoothSocket* clientsock = serversock.accept();
		cout << "accepted from " << clientsock->getForeignAddress().getAddress() << endl;
		MessageBox& mb = clientsock->getMessageBox();
			
		
		while(mb.isRunning()) {
			cout << "A(voor camera) of B(voor grijpen): "<<endl;
			input = mb.readMessage();  //blokkeert niet
			if(input != ""){
			cout <<endl<<"input: " << input << endl;
			}

			if(input=="A"){
			for(;;){
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
				if(leftWhite > rightWhite){
					cout << "Turn Right!" << endl;
					solidRight(200);
				} else {
					cout << "Turn Left!" << endl;
					solidLeft(200);
				}
		
			}else if(BP.get_sensor(PORT_2, Ultrasonic2) == 0 && Ultrasonic2.cm <= 10){
				stop();
				break;
			}

			else {
				fwd();
			}
			tipka = cv::waitKey(30);
	
    		}
	}
		else if(input=="B"){
			detect(Ultrasonic2);
		}
		else if(input=="C"){
			moreSpeed(IAmSpeed, bocht);
		}
		else if(input == "UP"){ 
			fwdDPS(IAmSpeed);
			control(Ultrasonic2,clientsock);
		}else if(input == "LEFT"){
			solidLeft(bocht);
		}else if(input == "RIGHT"){
			solidRight(bocht);
		}else if(input == "DOWN"){
			down(-IAmSpeed);
		}else if(input == "FIRE"){
			stop();
		}
		//doe andere dingen.
		cout << ".";
		cout.flush();
		usleep(500000); // wacht 500 ms
		}	
		clientsock->close();
   		 // the camera will be deinitialized automatically in VideoCapture destructor
    		return 0;
			
		}
	}
}

void exit_signal_handler(int signo){
  if(signo == SIGINT){
    BP.reset_all();    // Reset everything so there are no run-away motors
    exit(-2);
  }
}
