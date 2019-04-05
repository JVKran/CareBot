#include <iostream>
#include <unistd.h>
#include <fstream>
#include <vector>
#include <sstream>

using namespace std;

int main () {
  string line;
  vector<string> y(3);
  int j = 0;
  string tmp;
  ifstream myfile ("/dev/rfcomm0");
  if (myfile.is_open()){
    while ( getline (myfile,line) ){
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
      cout << "1: " << y[0] << " 2: " << y[1] << " 3: " << y[2] << endl;
    }
  } else {
      cout << "Unable to open file"; 
  }
  return 0;
}
