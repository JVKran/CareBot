#include "BluetoothSerial.h" //Header File for Serial Bluetooth, will be added by default into Arduino
#include <NewRemoteTransmitter.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>

#define TFT_CS        15
#define TFT_RST        4
#define TFT_DC         2

#define joyX          32
#define joyY          33

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

BluetoothSerial ESP_BT;
NewRemoteTransmitter actionTransmitter(54973440, 14, 261, 3);

int incoming;
String message;
int xValue;
int yValue;

void setup() {
  tft.initR(INITR_BLACKTAB);
  tft.fillScreen(ST77XX_BLACK);
  Serial.begin(9600); //Start Serial monitor in 9600
  ESP_BT.begin("CareBot"); //Name of your Bluetooth Signal
  Serial.println("Klaar om te verbinden!");
  pinMode (5, OUTPUT);//Specify that LED pin is output
  pinMode(32, joyX);
  pinMode(33, joyY);
}

void loop() {
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
  delay(2000);
  ESP_BT.print("Loop");
  xValue = analogRead(joyX);
  yValue = analogRead(joyY);
  Serial.print("X-waarde: ");
  Serial.print(xValue);
  Serial.print(", Y-waarde: ");
  Serial.print(yValue);
  Serial.println();
}

