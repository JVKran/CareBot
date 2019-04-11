#include <iostream>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <stdio.h>
#include <unistd.h>
#include "BrickPi3.h"
#include <signal.h>
#include <iomanip>

using namespace cv;
using namespace std;
BrickPi3 BP;

void exit_signal_handler(int signo);

void solidLeft(int speed=45){
	BP.set_motor_dps(PORT_B, speed*1.07);
        BP.set_motor_dps(PORT_C, -speed);
}

void solidRight(int speed=45){
	BP.set_motor_dps(PORT_B, -speed*1.07);
        BP.set_motor_dps(PORT_C, speed);
}

void fwd(int speed=45){
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
	BP.set_sensor_type(PORT_2, SENSOR_TYPE_NXT_ULTRASONIC);
	BP.set_motor_limits(PORT_D, 20, 0);
	BP.set_motor_limits(PORT_B, 0, 0);
	BP.set_motor_limits(PORT_C, 0, 0);
}

void down(int speed=-45){
	BP.set_motor_dps(PORT_B, speed); //ivm ongelijkheid motoren.
    	BP.set_motor_dps(PORT_C, speed);
}


 int main( int argc, char** argv )
 {
    signal(SIGINT, exit_signal_handler); // register the exit function for Ctrl+C
    BP.detect(); // Make sure that the BrickPi3 is communicating and that the firmware is compatible with the drivers.
	
    if(!initialize()){
		cout << "Initialisatie gelukt!";
     } else {
		cout << "Initialisatie mislukt...";
		BP.reset_all();
		exit(-2);
	}
	
    VideoCapture cap(0); //capture the video from web cam

    if ( !cap.isOpened() )  // if not success, exit program
    {
         cout << "Cannot open the web cam" << endl;
         return -1;
    }

    namedWindow("Control", CV_WINDOW_AUTOSIZE); //create a window called "Control"

  int iLowH = 0;//Hue (0 - 179)De kleur
 int iHighH = 179;

  int iLowS = 0; //Saturation (0 - 255)Hoeveel het met wit mixed
 int iHighS = 255;

  int iLowV = 255; //Value (0 - 255) Hoeveel het met zwart mixed
 int iHighV = 255;

  //Create trackbars in "Control" window
 cvCreateTrackbar("LowH", "Control", &iLowH, 179); //Hue (0 - 179)
 cvCreateTrackbar("HighH", "Control", &iHighH, 179);

  cvCreateTrackbar("LowS", "Control", &iLowS, 255); //Saturation (0 - 255)
 cvCreateTrackbar("HighS", "Control", &iHighS, 255);

  cvCreateTrackbar("LowV", "Control", &iLowV, 255); //Value (0 - 255)
 cvCreateTrackbar("HighV", "Control", &iHighV, 255);

    while (true)
    {
        Mat imgOriginal;

        bool bSuccess = cap.read(imgOriginal); // read a new frame from video

         if (!bSuccess) //if not success, break loop
        {
             cout << "Cannot read a frame from video stream" << endl;
             break;
        }

    	Mat imgHSV;

	 cvtColor(imgOriginal, imgHSV, COLOR_BGR2HSV); //Convert the captured frame from BGR to HSV
 
 	 Mat imgThresholded;

   	inRange(imgHSV, Scalar(iLowH, iLowS, iLowV), Scalar(iHighH, iHighS, iHighV), imgThresholded); //Threshold the image
      
  //morphological opening (remove small objects from the foreground)
  	erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );
  	dilate( imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) ); 

   //morphological closing (fill small holes in the foreground)
  	dilate( imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) ); 
 	erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );

   	imshow("Thresholded Image", imgThresholded); //show the thresholded image
  	imshow("Original", imgOriginal); //show the original image
	cout <<"Trash: "<<cv::countNonZero(imgThresholded) << endl;

	if(cv::countNonZero(imgThresholded) > 100){
		cv::Mat left = imgThresholded(cv::Range(0, imgThresholded.rows -1), cv::Range(0, imgThresholded.cols / 2 -1));
		cv::Mat right = imgThresholded(cv::Range(0, imgThresholded.rows -1), cv::Range(imgThresholded.cols / 2 + 1, imgOriginal.cols -1));
		int rightWhite = cv::countNonZero(right);
		int leftWhite = cv::countNonZero(left);
		cout << "links: "<<leftWhite << endl;
		cout << "rechts: "<<rightWhite << endl;
		if(rightWhite <= 500 ){
			//cout << "Turn left!" << endl;
			solidLeft(200);
		} else if(leftWhite <= 500){
			//cout << "Turn right!" << endl;
			solidRight(200);
			
		}else if(rightWhite>500 &&leftWhite>500){
			fwd(400);
			
		}
	}
	if(cv::countNonZero(imgThresholded) < 100){
		stop();
	}

        if (waitKey(30) == 27) //wait for 'esc' key press for 30ms. If 'esc' key is pressed, break loop
       {
            cout << "esc key is pressed by user" << endl;
            break; 
       }
    }

   return 0;

}

void exit_signal_handler(int signo){
  if(signo == SIGINT){
    BP.reset_all();    // Reset everything so there are no run-away motors
    exit(-2);
  }
}
