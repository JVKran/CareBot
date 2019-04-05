#include <opencv2/opencv.hpp>
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <iomanip>
#include <vector>
#include <fstream>


using namespace cv;
using namespace std;

int main(){
string line;
string inputFD;//Forward Down
string inputLR;//Left Right
string inputB;//Button

vector<string> y(2);
ifstream myfile ("/dev/rfcomm0");

if(myfile.is_open()){
	while(getline (myfile, line)){
		cout << line << endl;
		string tmp;
		int j = 0;

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
		cout << "1: " << y[0] << " 2: " << y[1] << " 3: " << y[2] << endl;
	}
	myfile.close();
}else{
     	cout << "Unable to open" << endl;
     }
}
