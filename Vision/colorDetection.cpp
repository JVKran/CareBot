#include <opencv2/opencv.hpp>
#include <iostream>
#include <stdio.h>
#include <unistd.h> 


using namespace cv;
using namespace std;


int ct = 0; 
char tipka;
char filename[100]; // For filename
int  c = 1; // For filename

Mat redFilter(const Mat& src){
    assert(src.type() == CV_8UC3);

    Mat redOnly;
    inRange(src, Scalar(0, 0, 0), Scalar(125, 125, 255), redOnly);

    return redOnly;
}

int main(int, char**){


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
	if(cv::countNonZero(redOnly) > 50000){
		cv::Mat left = redOnly(cv::Range(0, redOnly.rows -1), cv::Range(0, redOnly.cols / 2 -1));
		cv::Mat right = redOnly(cv::Range(0, redOnly.rows -1), cv::Range(redOnly.cols / 2 + 1, redOnly.cols -1));
		int rightWhite = cv::countNonZero(right);
		int leftWhite = cv::countNonZero(left);
		if(leftWhite > rightWhite){
			cout << "Turn Right!" << endl;
		} else {
			cout << "Turn Left!" << endl;
		}
	}	



        tipka = cv::waitKey(30);

	if(tipka == 'q'){
            cv::waitKey(10); 

            imshow("CAMERA 1", frame);
            imwrite("test.jpg", frame);
            cout << "Frame_" << c << endl;
            c++;
	}

        if (tipka == 'a') {
            cout << "Terminating..." << endl;
            usleep(2000000);
            break;
        }


    }
    // the camera will be deinitialized automatically in VideoCapture destructor
    return 0;
}
