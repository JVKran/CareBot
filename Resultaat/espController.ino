#include "BluetoothSerial.h" //Header File for Serial Bluetooth, will be added by default into Arduino
#include <NewRemoteTransmitter.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>

// Definieer TFT display pins
#define TFT_CS        15
#define TFT_RST        4
#define TFT_DC         2

// Definieer analoge joystick pins
#define joyY      A6
#define joyX      A7
#define joyButton 14

//Public Variabelen in verband met tijdsnood
int xValue;
int yValue;
int bValue;
int pos = 0; //Used for current choice of marker
bool menu = true; //Display marker or not
String action3;
bool manual;

// Initialiseer 1,77 inch Display
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

//Initialiseer serieele bluetooth library
BluetoothSerial ESP_BT;
//Initialiseer 433mhz zender
NewRemoteTransmitter actionTransmitter(54973440, 16, 261, 3);

int incoming;
String message;
bool notifyNext = false;

//Verplaats de navigatiepijl naar de plaats van pos als er een menu venster is geopend
void choiceMarker(int pos, String color = "white"){
  if(menu){
    if(color == "black"){
      tft.setTextColor(ST77XX_BLACK);
    } else {
      tft.setTextColor(ST77XX_WHITE);
    }
    tft.setCursor(4,pos*10+5);
    tft.print("->");
  }
}

void setup() {
  tft.initR(INITR_BLACKTAB);
  tft.fillScreen(ST77XX_BLACK);
  tft.fillCircle(115, 7, 3, ST77XX_RED);
  choiceMenu();
  choiceMarker(pos);
  Serial.begin(9600); //Start Serial monitor in 9600
  ESP_BT.begin("CareBot"); //Name of your Bluetooth Signal
  Serial.println("Klaar om te verbinden!");
  pinMode (5, OUTPUT);//Specify that LED pin is output
  pinMode(joyButton, INPUT);
  digitalWrite(joyButton, HIGH);
}

// Lees de inkomende bluetoothberichten en voer acties uit op basis hiervan
void handleIncomingBTMessages(void){
  if (ESP_BT.available()) //Check if we receive anything from Bluetooth
  {
    tft.fillCircle(115, 7, 3, ST77XX_GREEN);
    incoming = ESP_BT.read(); //Read what we recevive 
    if(incoming!='#'){
      message.concat(char(incoming));   
    } else {
      if(message.indexOf("ghtOn") > 0){
        ESP_BT.println("Licht aan!");
        actionTransmitter.sendUnit(3, true);
        action3 = "on";
        if(menu){
          tft.setCursor(65,25);
          tft.setTextColor(ST77XX_GREEN);
          tft.print("Licht");
        }
      } else if(message.indexOf("ightOf") > 0){
        ESP_BT.println("Licht uit!");
        actionTransmitter.sendUnit(3, false);
        action3 = "off";
        if(menu){
          tft.setCursor(65,25);
          tft.setTextColor(ST77XX_RED);
          tft.print("Licht");
        }
        //Print rode balken op plaats van voorwerpen
      } else if(message.indexOf("eftDang") > 0 && manual == true){
          tft.fillRect(2,30,5,100,ST77XX_RED);
      } else if(message.indexOf("ghtDang") > 0 && manual == true){
          tft.fillRect(122,30,5,100,ST77XX_RED);
      } else if(message.indexOf("oFrontDang") > 0 && manual == true){
          tft.fillRect(10,15,110,5,ST77XX_BLACK);
          Serial.println("balk front weg");
      } else if(message.indexOf("ontDang") > 0 && manual == true){
          tft.fillRect(10,15,110,5,ST77XX_RED);
      } else if(message.indexOf("oDirectionDan") > 0 && manual == true){
          tft.fillRect(2,30,5,100,ST77XX_BLACK);
          tft.fillRect(122,30,5,100,ST77XX_BLACK);
      }
      Serial.println(message);
      message="";
    }
  }
}

//Verplaats de keuzepijl als de waarden groter zijn dan het minimum en start een functie als de knop wordt ingedrukt
//Als het ware de basis van het hele menu.
void moveChoiceMarker(int x, int y, int b){
  if(y > 3600){
    if(pos > 0){
      choiceMarker(pos, "black");
      pos--;
      choiceMarker(pos, "white");
    }
    while(y > 3600){
      delay(200);
      y = analogRead(joyY);
      Serial.println(yValue);
    }
  } else if (y < 500){
    if(pos < 2){
      choiceMarker(pos, "black");
      pos++;
      choiceMarker(pos, "white");
    }
    while(y < 500){
      delay(200);
      y = analogRead(joyY);
      Serial.println(yValue);
    }
  }
  if(b == 0){
    if(pos == 0){
      if(notifyNext){
        ESP_BT.println("4");
        notifyNext = false;
      }
      menu = false;
      manualControl();
    } else if(pos == 1){
      if(notifyNext){
        ESP_BT.println("5");
        notifyNext = false;
      }
      menu = false;
      autoControl();
    } else if(pos == 2){
      if(action3 == "off"){
        actionTransmitter.sendUnit(3, true);
        action3 = "on";
        tft.setCursor(65,25);
        tft.setTextColor(ST77XX_GREEN);
        tft.print("Licht");
      } else {
        actionTransmitter.sendUnit(3, false);
        action3 = "off";
        tft.setCursor(65,25);
        tft.setTextColor(ST77XX_RED);
        tft.print("Licht");
      }
    }
  }
}

//Druk het keuzemenu af met "Licht" in rood of groen afhankelijk van staat
void choiceMenu(){
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(20,5);
  tft.print("Handmatig");
  tft.setCursor(20,15);
  tft.print("Automatisch");
  tft.setCursor(20,25);
  tft.print("Schakel");
  tft.setCursor(65,25);
  if(action3 == "on"){
    tft.setTextColor(ST77XX_GREEN);
  } else {
    tft.setTextColor(ST77XX_RED);
  }
  tft.print("Licht");
  tft.setTextColor(ST77XX_WHITE);
}

//Automatische besturing verstuurd "Keep Alive" om verbinding actief te houden. Daarnaast kan het worden gestopt door de joystickknop ingedrukt te houden
void autoControl(void){
  delay(1000);
  int counter = 0;
  tft.fillScreen(ST77XX_BLACK);
  tft.fillCircle(115, 7, 3, ST77XX_GREEN);
  manual = true;
  while(true){
    bValue = digitalRead(joyButton);
    delay(75);
    while(bValue == 0){
      bValue = digitalRead(joyButton);
      delay(100);
      counter++;
    }
    if(counter < 10 && counter >= 1){
      counter = 0;
    } else if(counter == 0){
    } else {
      tft.fillScreen(ST77XX_BLACK);
      menu = true;
      manual = false;
      choiceMenu();
      pos = 0;
      choiceMarker(pos);
      notifyNext = true;
      break;
    }
    ESP_BT.println("Keep alive");
    delay(50);
  }
}

//Handmatige besturing stuurt de rauwe x, y en knop waarden door naar de Raspberry Pi
void manualControl(void){
  delay(1000);
  int counter = 0;
  tft.fillScreen(ST77XX_BLACK);
  tft.fillCircle(115, 7, 3, ST77XX_GREEN);
  manual = true;
  while(true){
    tft.setTextColor(ST77XX_BLACK);;
    tft.setCursor(10,135);
    tft.print(int(float(100)/float(4095)*yValue)-44);
    tft.print('%');
    handleIncomingBTMessages();
    xValue = analogRead(joyX);
    yValue = analogRead(joyY);
    bValue = digitalRead(joyButton);
    tft.setTextColor(ST77XX_WHITE);;
    tft.setCursor(10,135);
    tft.print(int(float(100)/float(4095)*yValue)-44);
    tft.print('%');
    ESP_BT.print(yValue);
    ESP_BT.print(",");
    ESP_BT.print(xValue);
    ESP_BT.print(",");
    delay(50);
    while(bValue == 0){
      bValue = digitalRead(joyButton);
      delay(100);
      counter++;
    }
    if(counter < 10 && counter >= 1){
      ESP_BT.println(0);
      counter = 0;
    } else if(counter == 0){
      ESP_BT.println(1);
    } else {
      tft.fillScreen(ST77XX_BLACK);
      ESP_BT.println(1);
      menu = true;
      manual = false;
      choiceMenu();
      pos = 0;
      choiceMarker(pos);
      notifyNext = true;
      break;
    }
    tft.fillRect(10,150,110, 5,ST77XX_BLACK);
    tft.fillRect(10,150,float(110)/float(4095)*xValue, 5,ST77XX_BLUE);
  }
}

//Doe dit zo vaak als maar kan
void loop() {
  handleIncomingBTMessages();
  delay(20);
  xValue = analogRead(joyX);
  yValue = analogRead(joyY);
  bValue = digitalRead(joyButton);
  moveChoiceMarker(xValue, yValue, bValue);
  Serial.println(pos);
  Serial.print(xValue);
  Serial.print(" ");
  Serial.println(yValue);
}

