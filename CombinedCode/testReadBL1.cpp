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

string line;
string inputFD;//Forward Down
string inputLR;//Left Right
string inputB;//Button

int main(){
std::ifstream myfile("/home/pi/dev/rfcomm0.txt");

if(input.is_open()){
	while(getline (myfile, line)){
		for(unsigned int  =0; i<line.size();i++){
				if(line[-1]=="0"){
					inputB="A";
					break;
				}
				else if(i<3){
					if(line[i]!=","){
						inputFD+=line[i];
						}
					}
				else{
					if(line[i]!=","){
						inputLR+=line[i]
						}
					}
				break;
			}
		}
		myfile.close();
		}else{
     		cout << "Unable to open" << endl;
     		}
}
//9 max waarde, 3 min waarde
