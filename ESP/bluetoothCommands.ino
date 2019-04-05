#include "BluetoothSerial.h" //Header File for Serial Bluetooth, will be added by default into Arduino
#include <NewRemoteTransmitter.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>

#define TFT_CS        15
#define TFT_RST        4
#define TFT_DC         2

#define joyY      A6
#define joyX      A7
#define joyButton 32

int xValue;
int yValue;
int bValue;
int pos = 0; //Used for current choice of marker

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

BluetoothSerial ESP_BT;
NewRemoteTransmitter actionTransmitter(54973440, 14, 261, 3);

int incoming;
String message;

void choiceMarker(int pos, String color = "white"){
  if(color == "black"){
    tft.setTextColor(ST77XX_BLACK);
  } else {
    tft.setTextColor(ST77XX_WHITE);
  }
  tft.setCursor(4,pos*10+5);
  tft.print("->");
}


void setup() {
  tft.initR(INITR_BLACKTAB);
  tft.fillScreen(ST77XX_BLACK);
  choiceMenu();
  choiceMarker(pos);
  Serial.begin(9600); //Start Serial monitor in 9600
  ESP_BT.begin("CareBot"); //Name of your Bluetooth Signal
  Serial.println("Klaar om te verbinden!");
  pinMode (5, OUTPUT);//Specify that LED pin is output
  pinMode(joyButton, INPUT);
  digitalWrite(joyButton, HIGH);
}

void handleIncomingBTMessages(void){
  if (ESP_BT.available()) //Check if we receive anything from Bluetooth
  {
    incoming = ESP_BT.read(); //Read what we recevive 
    if(incoming!='#'){
      message.concat(char(incoming));   
    } else {
      if(message=="lightOn"){
        ESP_BT.println("Licht aan!");
        actionTransmitter.sendUnit(3, true);
      } else if(message=="lightOff"){
        ESP_BT.println("Licht uit!");
        actionTransmitter.sendUnit(3, false);
      } else {
        ESP_BT.print(message);
        ESP_BT.print(" is een onbekend commando.");
      }
      Serial.println(message);
      message="";
    }
  }
}

void moveChoiceMarker(int x, int y, int b){
  if(y > 3600){
    if(pos > 0){
      choiceMarker(pos, "black");
      pos--;
    }
    while(y > 3600){
      delay(200);
      y = analogRead(joyY);
      Serial.println(yValue);
    }
  } else if (y < 500){
    if(pos < 1){
      choiceMarker(pos, "black");
      pos++;
    }
    while(y < 500){
      delay(200);
      y = analogRead(joyY);
      Serial.println(yValue);
    }
  }
  if(b == 1){
    if(pos == 0){
      manualControl();
    } else if(pos == 1){
      automaticControl();
    }
  }
  choiceMarker(pos, "white");
  Serial.println("loopje");
}

void choiceMenu(){
  // Handmatige besturing
  // Automatische besturing
  tft.setCursor(20,5);
  tft.print("Handmatig");
  tft.setCursor(20,15);
  tft.print("Automatisch");
}

void manualControl(void){
  
}

void automaticControl(void){
  
}

void loop() {
  handleIncomingBTMessages();
  delay(20);
  xValue = analogRead(joyX);
  yValue = analogRead(joyY);
  bValue = digitalRead(joyButton);
  moveChoiceMarker(xValue, yValue, bValue);
  Serial.println(pos);
}

