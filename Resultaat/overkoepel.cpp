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
int sc4(){
	
	pid_t pid = fork();
	if(pid ==0){
		char *args[] = {};
		execv("./manControl",args);
	}
	else if(pid >0){
		int exit_status;
		wait(&exit_status);
		return(WEXITSTATUS(exit_status));
	}
}
int sc5(){
	pid_t pid = fork();
	if(pid ==0){
		char *args[] = {};
		execv("./autoControl",args);
	}
	else if(pid >0){
		int exit_status;
		wait(&exit_status);
		return(WEXITSTATUS(exit_status));
	}
}
int sc6(){
	pid_t pid = fork();
	if(pid ==0){
		char *args[] = {(char*) "/usr/share/opencv/haarcascades/haarcascade_frontalface_default.xml", (char*) "/home/pi/piprograms/Camera/Face/trainFace.txt", char(0) };
		execv("/faceRec",args);
	}
	else if(pid >0){
		int exit_status;
		wait(&exit_status);
		return(WEXITSTATUS(exit_status));
	}
}
int sc7(){
	pid_t pid = fork();
	if(pid ==0){
		char *args[] = {};
		execv("./ledCamera",args);
	}
	else if(pid >0){
		int exit_status;
		wait(&exit_status);
		return(WEXITSTATUS(exit_status));
	}
}
int sc8(){
	pid_t pid = fork();
	if(pid ==0){
		char *args[] = {};
		execv("./info",args);
	}
	else if(pid >0){
		int exit_status;
		wait(&exit_status);
		return(1);
	}
}


int main(){
int exit_status=4;

while(1){
cout <<"status            " << exit_status<<endl;
if(exit_status==4){exit_status = sc4();}
else if(exit_status ==5){cout <<"code 5"<<endl;exit_status = sc5();}
else if(exit_status ==6){cout<<"code 6"<<endl;exit_status = sc6();}
else if(exit_status ==7){exit_status = sc7();}
else{sc8();return(1);}
}

}
