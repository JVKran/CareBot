#include <string>
#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>


using namespace std;
int sc1(string uittevoeren){
	pid_t pid = fork();
	if(pid ==0){
		char *args[] = {};
		execv("uittevoeren",args);
	}
	else if(pid >0){
	int exit_status;
	wait(&exit_status);
	return(exit_status);
	}
}
int main(){
int exit_status=6;

while(1){
if(exit_status==4){sc1("./autoControl");}
else if(exit_status ==5){sc1("./faceRec");}
else if(exit_status ==6){sc1("./completeManualControl");}
else if(exit_status ==7){sc1("./grijper");}
else{sc1("./prog1");}
}

}
