//dit programma zal, afhankelijk van de exitstatus van het vorige programma een nieuw programma openen, het eerste programma is altijd mancontrol
//we gebruiken exitcodes 4,5,6,7 voor respectievelijk de programma's manControl, autoControl,faceRec en ledCamera. 
//indien er een andere exitcode uitkomt wordt het programma stop aangeroepen, 
//die stopt alle moteren direct, dit omdat een script dat crasht die niet per se zelf doet, hierna stopt overkoepel.cpp

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
	
	pid_t pid = fork();			//forkt het proces
	if(pid ==0){
		char *args[] = {};
		execv("./manControl",args);	//child start volgend proces op
	}
	else if(pid >0){
		int exit_status;
		wait(&exit_status);			//parent wacht op dood van kind
		return(WEXITSTATUS(exit_status));	//exitstatus wordt naar interperteerbaar formaat omgezet en gereturned
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
		execv("./stop",args);
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
cout <<"status            " << exit_status<<endl;	//geeft aan welk programma geopend gaat worden
if(exit_status==4){exit_status = sc4();}	//roept functie 4 aan als de exitcode 4 is, zelfde maar andere nummers voor volgende functies	
else if(exit_status ==5){cout <<"code 5"<<endl;exit_status = sc5();}
else if(exit_status ==6){cout<<"code 6"<<endl;exit_status = sc6();}
else if(exit_status ==7){exit_status = sc7();}
else{sc8();return(1);} // roep bij onverwachte exitcode stop aan en stop overkoepel.cpp
}

}
