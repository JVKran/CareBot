#include <iostream>
#include <unistd.h>
#include <fstream>


using namespace std;

int main () {
  string line;
  ifstream myfile ("/dev/rfcomm0");
  if (myfile.is_open()){
    while ( getline (myfile,line) ){
      cout << line << endl;
    }
  } else {
      cout << "Unable to open file"; 
  }
  return 0;
}
