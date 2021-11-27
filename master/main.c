// #pragma GCC optimize ("-o0")
// #pragma GCC push_options

#include <Arduino.h>
#include <Wire.h> 
#include <LiquidCrystal.h>
#include <stdlib.h>
#include <stdio.h>

#define SEND_HUMIDITY_TEMP_COMMAND "<SEND_HT>"

// Function Declarations
void processData();
void recieveBluetoothData();
void sendBluetoothCommand();

const byte maxDataLength = 20;          // maxDataLength is the maximum length allowed for received data.
char receivedChars[maxDataLength+1] ;
boolean newData = false;               // newData is used to determine if there is a new data

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

const int BLUETOOTH_RATE = 9600;


int outputValue = 255;
int analogOutPin1 = 7;
int analogOutPin2 = 6;
int analogOutPin3 = 10;


void setup() {
  // set up the LCD's number of columns and rows:
  lcd.begin(20, 4);
  
  // We use Serial port for bluetooth module :
  Serial.begin(BLUETOOTH_RATE);

  pinMode(analogOutPin1, OUTPUT);
  pinMode(analogOutPin2, OUTPUT);
  pinMode(analogOutPin3, OUTPUT);

  analogWrite(analogOutPin1, 255);
  analogWrite(analogOutPin2, 0);

  delay(300);

}

void loop() {
  // sendBluetoothCommand();
  // delay(500);
  recieveBluetoothData();                // check to see if we have received any new data
  if (newData)  {   processData();  }
}


void processData() {
    newData = false;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.println(receivedChars);

    char TempChars[10] ;
    char HumidityChars[10] ;
    for(int i=0; i<11; i++) {
      if (receivedChars[i] == ':') {
        strncpy(TempChars, receivedChars, i);
        TempChars[i] = '\0';
        strcpy(HumidityChars, &receivedChars[i+1]);
      }
    }
    float ftemp = atof(TempChars);
    float fhumidity = atof(HumidityChars);

    lcd.setCursor(0, 1);
    if(fhumidity > 50) {
      analogWrite(analogOutPin3, 0);
      lcd.println("Not Watering");
    }
    else if(fhumidity < 20) {
      analogWrite(analogOutPin3, 64);
      lcd.println("Watering 20 CC/min");
    }
    else {
      if(ftemp < 25) {
        analogWrite(analogOutPin3, 0);
        lcd.println("Not Watering");
      }
      else {
        analogWrite(analogOutPin3, 25);
        lcd.println("Watering 10 CC/min");
      }
    }
}
 
void recieveBluetoothData() {
     static boolean recvInProgress = false;
     static byte ndx = 0;
     char startMarker = '<';
     char endMarker = '>';
 
     if (Serial.available() > 0) {
          char rc = Serial.read();
          if (recvInProgress == true) {
               if (rc != endMarker) {
                    if (ndx < maxDataLength) { receivedChars[ndx] = rc; ndx++;  }
               }
               else {
                     receivedChars[ndx] = '\0'; // terminate the string
                     recvInProgress = false;
                     ndx = 0;
                     newData = true;
               }
          }
          else if (rc == startMarker) { recvInProgress = true; }
     }
}

void sendBluetoothCommand() {
  delay(5000);
  Serial.write(SEND_HUMIDITY_TEMP_COMMAND);
}