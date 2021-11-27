// #pragma GCC optimize ("-o0")
// #pragma GCC push_options

#include <Arduino.h>
#include <Wire.h> 
#include <string.h>
#include <stdio.h>

#define START_MARKER '<'
#define END_MARKER '>'
#define DELIMETER ':'
#define MAX_DATA_LENGTH 100
#define NULL_CHAR '\0'
#define humidityCommand 0xF5
#define tempCommand 0xF3
#define SEND_HUMIDITY_TEMP_COMMAND "SEND_HT"
#define Addr 0x40

bool hasIncommingData = false;
char receivedChars[MAX_DATA_LENGTH + 1];
float temperature = 0;
float humidity = 0;

void recvCommand() {
  delay(5000);
  strcpy(receivedChars, SEND_HUMIDITY_TEMP_COMMAND);
  hasIncommingData = true;
  return;
  static boolean recvInProgress = false;
  static byte index = 0;

  if (Serial.available() > 0) {
    char recivedChar = Serial.read();
    if (recvInProgress == true) {
        if (recivedChar != END_MARKER) {
          if (index < MAX_DATA_LENGTH) {
            receivedChars[index] = recivedChar;
            index++;
          }
        } 
        else {
            receivedChars[index] = NULL_CHAR;
            recvInProgress = false;
            index = 0;
            hasIncommingData = true;
        }
    }
    else if (recivedChar == START_MARKER) {
        recvInProgress = true;
    }
  }
}

void generateTHResponse(char* response) {
  char temperatureString[10];
  char humidityString[10];
  
  dtostrf(temperature, 3,2, temperatureString);
  dtostrf(humidity, 3,2, humidityString);

  response[0] = START_MARKER;
  strcat(response, temperatureString);
  response[strlen(response)] = DELIMETER;
  strcat(response, humidityString);
  // response[strlen(response)] = END_MARKER;
  response[strlen(temperatureString)+strlen(humidityString)+2-1] = END_MARKER;
  response[strlen(temperatureString)+strlen(humidityString)+3-1] = NULL_CHAR;
}

void sendData() {
  char response[MAX_DATA_LENGTH + 1];
  generateTHResponse(response);
  // Serial.println("response here:");
  //Serial.println(response);
  Serial.write(response);
  // Serial.write(START_MARKER);
  // Serial.write(int(humidity));
  // Serial.write(DELIMETER);
  // Serial.write(int(temperature));
  // Serial.write(END_MARKER);
}

void processCommand() {
  hasIncommingData = false;
  // Serial.println(receivedChars);
  if (strcmp (receivedChars, SEND_HUMIDITY_TEMP_COMMAND) == 0) {
    // Serial.println("in process");
    sendData();
  }
}

void setup() {  
  Wire.begin();
  Serial.begin(9600);
  delay(300);
}

void requestDataFromSensor(int commandNum) {
    Wire.beginTransmission(Addr);  
    Wire.write(commandNum);  
    Wire.endTransmission();
}

float readHumidity() {
  unsigned int data[2]; 
  data[0] = Wire.read();    
  data[1] = Wire.read();
  float humidity = (((data[0] * 256.0 + data[1]) * 125.0) / 65536.0) - 6;
  return humidity;
}

float readTemp() {
  unsigned int data[2]; 
  data[0] = Wire.read();    
  data[1] = Wire.read(); 
  float cTemp = (((data[0] * 256.0 + data[1]) * 175.72) / 65536.0) - 46.85;    
  return cTemp; 
}


void loop() {  
  unsigned int data[2];  
  requestDataFromSensor(humidityCommand);
  delay(500);
  Wire.requestFrom(Addr, 2);  
  if(Wire.available() == 2)  
  {
    humidity = readHumidity();
    // Serial.print("H: ");
    // Serial.print(humidity);    
    // Serial.print(" %RH");  
  }
  requestDataFromSensor(tempCommand);
  delay(500);
  Wire.requestFrom(Addr, 2);
  if(Wire.available() == 2) {    
    temperature = readTemp();
    // Serial.print("T: ");    
    // Serial.print(temperature);
    // Serial.println(" C");  
  }  
  
  recvCommand();
  if (hasIncommingData)  {
    processCommand(); 
  }   
  delay(300);

}

