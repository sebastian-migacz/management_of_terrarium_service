// Import required libraries
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESP8266HTTPClient.h>
#include <FS.h>
#include <OneWire.h>
#include <Hash.h>
#include <time.h>

//define output pins
#define GROWLIGHT2PIN D5
#define GROWLIGHTPIN D4
#define SPRINKLERPIN D7
#define WATERFALLPIN D6

//variables definition
String lightgrow1state, lightgrow2state, sprinklerstate;
byte counter = 0, spStatus=2, spHum=38, wfStatus=2, glStatus=1, rgbr=1, rgbg=2, rgbb=3, evSunrise=1, evSunset=2, evRain=1;
byte StatusSettingsArray[8], TimeSettingsArray[8][3], RGBSettinsArray[3];
char controlBuffor[50];

const char* ssid = "kensaj 2,4";
const char* password = "******";
String apiKeyValue = "tPmAT5Ab3j7F9";

String sensorName = "DS18B20";
String sensorLocation = "Test";
float sensor1temp, sensor2temp;


int timezone = 7200;
int dst =0;

const char kHostname[] = "http://smartterra.000webhostapp.com";
const char kPath[] = "/recive-esp-settings.php";
const char* InsertDataScriptUrl = "http://smartterra.000webhostapp.com/post-esp-data.php";
const char* UpdateSettingsScriptUrl = "http://smartterra.000webhostapp.com/post-esp-settings.php";
const char* ReciveSettingsScriptUrl ="http://smartterra.000webhostapp.com/recive-esp-settings.php";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

//create OneWire interface
OneWire  ds(4);

void initializePin(){

  pinMode(D0, OUTPUT);
  pinMode(GROWLIGHTPIN, OUTPUT);
  pinMode(GROWLIGHT2PIN, OUTPUT);
  pinMode(WATERFALLPIN, OUTPUT);
  pinMode(SPRINKLERPIN, OUTPUT);
  pinMode(D8, OUTPUT);

  digitalWrite(GROWLIGHT2PIN, HIGH);
  digitalWrite(GROWLIGHTPIN, HIGH);
  digitalWrite(WATERFALLPIN, HIGH);
  digitalWrite(SPRINKLERPIN, HIGH);
  digitalWrite(D8, HIGH);
  digitalWrite(D0, HIGH);
}

/*==========================================================================================Temperature Measuremet====================
======================================================================================================================================*/

void tempMeasurement(){
  byte i, present = 0, type_s, data[12], addr[8];
  float celsius;
  
  if ( !ds.search(addr)) {
    ds.reset_search();
    delay(250);
    return;
  }
  if (OneWire::crc8(addr, 7) != addr[7]) {
      Serial.println("CRC is not valid!");
      return;
  }
 
  switch (addr[0]) {
    case 0x10:
      Serial.println("  Chip = DS18S20");  // or old DS1820
      type_s = 1;
      break;
    case 0x28:
      Serial.println("  Chip = DS18B20");
      type_s = 0;
      break;
    case 0x22:
      Serial.println("  Chip = DS1822");
      type_s = 0;
      break;
    default:
      Serial.println("Device is not a DS18x20 family device.");
      return;
  } 

  ds.reset();
  ds.select(addr);
  ds.write(0x44, 1);        // start conversion
  
  delay(1000);

  
  present = ds.reset();
  ds.select(addr);    
  ds.write(0xBE);         // Read Scratchpad

  for ( i = 0; i < 9; i++) {           //9 bytes
  data[i] = ds.read();
  }

  int16_t raw = (data[1] << 8) | data[0];
  if (type_s) {
    raw = raw << 3; // 9 bit resolution default
    if (data[7] == 0x10) {
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  }
  else {
    byte cfg = (data[4] & 0x60);
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
  }
  celsius = (float)raw / 16.0;
  Serial.print("  Temperature = ");
  Serial.print(celsius);
  Serial.print(" Celsius, ");
  

  switch (addr[1]) {
    
    case 46:
      sensor1temp = celsius;
      break;
      
    case 41:
      sensor2temp = celsius;   
      break;
    case 10:
      sensor1temp = celsius;
      break;
    default:
      Serial.println("Device is not listed");
      Serial.print("  :: ");
      Serial.print(addr[1]);
      return;
  }
}

String getTemperature1() {
  Serial.println(sensor1temp);
  return String(sensor1temp);
}

String getTemperature2() {
  Serial.println(sensor2temp);
  return String(sensor2temp);
}

/*==========================================================================================Return String from TimeArray================
======================================================================================================================================*/
String getStringfromArrayTime(byte index){
  String timeString;
  for(int i =0;i<3;i++){
    if(TimeSettingsArray[index][i]>9)
      timeString += TimeSettingsArray[index][i];
    else{
      timeString += 0;
      timeString += TimeSettingsArray[index][i];
    }
  }
  Serial.println(timeString);
  return timeString;
}

/*==========================================================================================SENDING DATA to DataBase==================
======================================================================================================================================*/

void sendDataToDatabase(){
    if(WiFi.status()== WL_CONNECTED){
    HTTPClient http;

  http.begin(InsertDataScriptUrl);
  
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  String httpRequestData = "api_key=" + apiKeyValue + "&sensor=" + sensorName
                          + "&location=" + sensorLocation + "&value1=" + String(getTemperature1())
                          + "&value2=" + String("null") + "&value3=" + String("null") + "";

   // String httpRequestData = "api_key=tPmAT5Ab3j7F9&sensor=BME280&location=Office&value1=24.75&value2=49.54&value3=1005.14";
    Serial.print("httpRequestData: ");
    Serial.println(httpRequestData);

    int httpResponseCode = http.POST(httpRequestData);
    if (httpResponseCode>0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
    }
    else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }
    // Free resources
    http.end();
  }
  else {
    Serial.println("WiFi Disconnected");
  }
}

/*==========================================================================================SENDING SETTINGS to DataBase==============
======================================================================================================================================*/
void sendSettingsToDatabase(){
    if(WiFi.status()== WL_CONNECTED){
      short id=2;
      glStatus=!digitalRead(GROWLIGHTPIN);
      spStatus=!digitalRead(SPRINKLERPIN);
      wfStatus=!digitalRead(WATERFALLPIN);
      HTTPClient http;
  
       http.begin(UpdateSettingsScriptUrl);
  
       http.addHeader("Content-Type", "application/x-www-form-urlencoded");

       String httpRequestData = "&id=" + String(id) 
                          + "&sprinklerstatus=" + String(StatusSettingsArray[0])
                          + "&sprinklerhum=" + String(StatusSettingsArray[1])
                          + "&waterfallstatus=" + String(StatusSettingsArray[2])
                          + "&waterfallontime=" + String(getStringfromArrayTime(0))
                          + "&waterfallofftime=" + String(getStringfromArrayTime(1)) 
                          + "&growlightstatus=" + String(StatusSettingsArray[3]) 
                          + "&growlightontime=" + String(getStringfromArrayTime(2)) 
                          + "&growlightofftime=" + String(getStringfromArrayTime(3)) 
                          + "&rgbr=" + String(RGBSettinsArray[0]) 
                          + "&rgbg=" + String(RGBSettinsArray[1]) 
                          + "&rgbb=" + String(RGBSettinsArray[2]) 
                          + "&eventsunrise=" + String(StatusSettingsArray[4]) 
                          + "&sunrisetime=" + String(getStringfromArrayTime(4)) 
                          + "&eventsunset=" + String(StatusSettingsArray[5]) 
                          + "&sunsettime=" + String(getStringfromArrayTime(5)) 
                          + "&eventrain=" + String(StatusSettingsArray[6]) 
                          + "&rainfrequency=" + String(getStringfromArrayTime(6)) 
                          + "";

      Serial.print("httpRequestData: ");
      Serial.println(httpRequestData);

       int httpResponseCode = http.POST(httpRequestData);
       if (httpResponseCode>0) {
         Serial.print("HTTP Response code: ");
         Serial.println(httpResponseCode);
       }
      else {
         Serial.print("Error code: ");
         Serial.println(httpResponseCode);
      }
      // Free resources
      http.end();
   }
   else {
      Serial.println("WiFi Disconnected");
  }
}

/*==========================================================================================Reciving SETTINGS from DataBase============
======================================================================================================================================*/

void reciveStatusDataFromDatabse(){
  HTTPClient http;
  http.begin(ReciveSettingsScriptUrl);
  int httpCode = http.GET();
  if (httpCode > 0) {
     Serial.println("startedRequest ok");
     String S = http.getString();
     byte SLenght = S.length(), indexOfSettingsData = 0, indexTimeArray = 0;
     String input;
     Serial.println(S);
     Serial.println(SLenght);
     for(byte i=0;i<SLenght;i++){
      if(S[i]!='x'){
        if(S[i]!=':'){
            input += S[i];
        }
        else{
          switch(indexOfSettingsData){
            
              case 4:
                  
                  TimeSettingsArray[0][indexTimeArray] = input.toInt();
                  indexTimeArray++;
                  if(indexTimeArray > 1)
                    indexTimeArray = 0;
                    input = "";
                  break;
                  
              case 5:
                  TimeSettingsArray[1][indexTimeArray] = input.toInt();
                  indexTimeArray++;
                  if(indexTimeArray > 1)
                    indexTimeArray = 0;
                    input = "";
                  break;
                  
              case 7:
                  TimeSettingsArray[2][indexTimeArray] = input.toInt();
                  indexTimeArray++;
                  if(indexTimeArray > 1)
                    indexTimeArray = 0;
                    input = "";
                  break;
                  
              case 8:
                  TimeSettingsArray[3][indexTimeArray] = input.toInt();
                  indexTimeArray++;
                  if(indexTimeArray > 1)
                    indexTimeArray = 0;
                    input = "";
                  break;
                  
              case 13:
                  TimeSettingsArray[4][indexTimeArray] = input.toInt();
                  indexTimeArray++;
                  if(indexTimeArray > 1)
                    indexTimeArray = 0;
                    input = "";
                  break;
                  
              case 15:
                  TimeSettingsArray[5][indexTimeArray] = input.toInt();
                  indexTimeArray++;
                  if(indexTimeArray > 1)
                    indexTimeArray = 0;
                    input = "";
                  break;
                  
              case 17:
                  TimeSettingsArray[6][indexTimeArray] = input.toInt();
                  indexTimeArray++;
                  if(indexTimeArray > 1)
                    indexTimeArray = 0;
                    input = "";
                  break;
          }

        } 
       }
       else{
        switch(indexOfSettingsData){
          case 1:
              StatusSettingsArray[0] = input.toInt();
            break;
            
          case 2:
              StatusSettingsArray[1] = input.toInt();
            break;
            
          case 3:
              StatusSettingsArray[2] = input.toInt();
            break;
            
          case 6:
              StatusSettingsArray[3] = input.toInt();
            break;

          case 9:
              RGBSettinsArray[0] = input.toInt();
            break;
            
          case 10:
              RGBSettinsArray[1] = input.toInt();
            break;
            
          case 11:
              RGBSettinsArray[2] = input.toInt();
            break;
            
          case 12:
              StatusSettingsArray[4] = input.toInt();
            break;

          case 14:
              StatusSettingsArray[5] = input.toInt();
            break;
            
          case 16:
              StatusSettingsArray[6] = input.toInt();
            break;

          default:
            break;
        }
        //Serial.println(indexOfSettingsData);
        // Serial.println(input);
        indexOfSettingsData++;
        input = "";
     }
     }
     
  }
  Serial.println("TimeArrray: "); 
  for(int j =0;j<7;j++)
  {
    Serial.print(j);
    Serial.print("::");
    for(int i = 0;i<3;i++)
    {
      Serial.print(TimeSettingsArray[j][i]); 
      Serial.print("--");   
    }
    Serial.println();
  }
}

/*==========================================================================================Logic Unit Function=========================
======================================================================================================================================*/
void controlUnitFunction (){
  bool actualState[3];
  actualState[0] = digitalRead(SPRINKLERPIN);
  actualState[1] = digitalRead(WATERFALLPIN);
  actualState[2] = digitalRead(GROWLIGHTPIN);
  
  Serial.print(actualState[0]);
  Serial.print(actualState[1]);
  Serial.print(actualState[2]);
  Serial.print("STATE");

  time_t now = time(nullptr);
  struct tm* p_tm = localtime(&now);
  int timeInSec = (p_tm->tm_sec+(60*(p_tm->tm_min+(60*p_tm->tm_hour))));
  Serial.print(p_tm->tm_hour);
  Serial.print("/");
  Serial.print(p_tm->tm_min);
  Serial.print("/");
  Serial.print(p_tm->tm_sec);
  Serial.print("///");
  Serial.print(timeInSec);
  Serial.println("");
  switch(StatusSettingsArray[0]){ //-----------------------------------------------------------------------------------SPRINKLER

    case 0:
      if(actualState[0] != 1)
        digitalWrite(SPRINKLERPIN, HIGH);
      break;
    case 1:
      if(actualState[0] != 0){
        digitalWrite(SPRINKLERPIN, LOW);
        delay(3000);
        digitalWrite(SPRINKLERPIN, HIGH);
      }
      break;
    case 2:
    
      break;
  }

  switch(StatusSettingsArray[2]){ //-----------------------------------------------------------------------------------WATERFALL

    case 0:
      if(actualState[1] != 1)
        digitalWrite(WATERFALLPIN, HIGH);
      break;
    case 1:
      if(actualState[1] != 0)
        digitalWrite(WATERFALLPIN, LOW);
      break;
    case 2:
        int onTime = (TimeSettingsArray[0][2]+(60*(TimeSettingsArray[0][1]+(60*TimeSettingsArray[0][0]))));
        int offTime = (TimeSettingsArray[1][2]+(60*(TimeSettingsArray[1][1]+(60*TimeSettingsArray[1][0]))));

        if(offTime > onTime){
          if(timeInSec < offTime){
            if(timeInSec > onTime){
              digitalWrite(WATERFALLPIN, LOW);
              Serial.println(">> Turn waterfall low");
            }
            else{
              digitalWrite(WATERFALLPIN, HIGH);
              Serial.println(">> Turn waterfall high1");
            }
          }
          else{
            digitalWrite(WATERFALLPIN, HIGH);
            Serial.println(">> Turn waterfall high2");
          }
        }
        else{
        digitalWrite(WATERFALLPIN, HIGH);
        Serial.println(">> Turn waterfall high3");
        }
      Serial.println("--WATERFALL--AUTO--");
      Serial.print("/on-");
      Serial.print(onTime);
      Serial.print("/off-");
      Serial.print(offTime);
      Serial.print("/acc-");
      Serial.print(timeInSec);
      Serial.print("--// ACC STATE // - ");
      Serial.print(digitalRead(WATERFALLPIN));
      Serial.println("");   
      break;
  }

  switch(StatusSettingsArray[3]){ //-----------------------------------------------------------------------------------GROWLIGHT

    case 0:
      if(actualState[2] != 1)
        digitalWrite(GROWLIGHTPIN, HIGH);
      break;
    case 1:
      if(actualState[2] != 0)
        digitalWrite(GROWLIGHTPIN, LOW);
      break;
    case 2:
        int onTime = (TimeSettingsArray[2][2]+(60*(TimeSettingsArray[2][1]+(60*TimeSettingsArray[2][0]))));
        int offTime = (TimeSettingsArray[3][2]+(60*(TimeSettingsArray[3][1]+(60*TimeSettingsArray[3][0]))));

        if(offTime > onTime){
          if(timeInSec < offTime){
            if(timeInSec > onTime){
              digitalWrite(GROWLIGHTPIN, LOW);
              Serial.println(">> Turn growlight low");
            }
            else{
              digitalWrite(GROWLIGHTPIN, HIGH);
              Serial.println(">> Turn growlight high1");
            }
          }
          else{
            digitalWrite(GROWLIGHTPIN, HIGH);
            Serial.println(">> Turn growlight high2");
          }
        }
        else{
        digitalWrite(GROWLIGHTPIN, HIGH);
        Serial.println(">> Turn growlight high3");
        }
      Serial.println("--GROWLIGHT--AUTO--");
      Serial.print("/on-");
      Serial.print(onTime);
      Serial.print("/off-");
      Serial.print(offTime);
      Serial.print("/acc-");
      Serial.print(timeInSec);
      Serial.print("--// ACC STATE // - ");
      Serial.print(digitalRead(GROWLIGHTPIN));
      Serial.println("");
      break;
  }
}

/*==========================================================================================PROCESOR function=========================
======================================================================================================================================*/

String processor(const String& var){
  Serial.println(var);
  if(var == "STATEGL"){
    if(digitalRead(GROWLIGHTPIN)){
      lightgrow1state = "ON";
   
    }
    else{
      lightgrow1state = "OFF";
     
    }
    Serial.print(lightgrow1state);
    return lightgrow1state;
  }
  else if(var == "STATESP"){
    if(digitalRead(SPRINKLERPIN)){
      sprinklerstate = "ON";
      
    }
    else{
      sprinklerstate = "OFF";
     
    }
    Serial.print(sprinklerstate);
    return sprinklerstate;
  }
  else if(var == "STATEWF"){
    if(digitalRead(WATERFALLPIN)){
      lightgrow2state = "ON";
      
    }
    else{
      lightgrow2state = "OFF";
      
    }
    Serial.print(lightgrow2state);
    return lightgrow2state;
  }
  else if (var == "TEMPERATURE1"){
    return getTemperature1();
  }
  else if (var == "TEMPERATURE2"){
    return getTemperature2();
  }
}

/*==========================================================================================SETUP Time ==============================
======================================================================================================================================*/
void timeConfigFunction(){
   configTime(timezone, dst, "pool.ntp.org","time.nist.gov"); 
   Serial.print("\nWaiting for real time...");
   while(!time(nullptr)){
    Serial.print("*");
    delay(1000);
   }
   Serial.print("Time response...OK");
}

/*==========================================================================================SETUP MCU function =======================
======================================================================================================================================*/

void setup(){
  
  // Serial port for debugging purposes
  Serial.begin(115200);
  
  initializePin();
  
  // Initialize SPIFFS
  if(!SPIFFS.begin()){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());
  
  timeConfigFunction(); //Time Config -------------------

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });
  
  // Route to load style.css file
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/style.css", "text/css");
  });

  server.on("/temperature1", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", getTemperature1().c_str());
  });
  
  server.on("/temperature2", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", getTemperature2().c_str());
  });
  
  server.on("/growlight", HTTP_GET, [](AsyncWebServerRequest *request){
    if(digitalRead(GROWLIGHTPIN)){
    digitalWrite(GROWLIGHTPIN, LOW);
    digitalWrite(GROWLIGHT2PIN, LOW);
    }
    else{
    digitalWrite(GROWLIGHTPIN, HIGH);
    digitalWrite(GROWLIGHT2PIN, HIGH);
    }   
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });
  
  server.on("/waterfall", HTTP_GET, [](AsyncWebServerRequest *request){
    if(digitalRead(WATERFALLPIN)){
    digitalWrite(WATERFALLPIN, LOW);
    }
    else{
    digitalWrite(WATERFALLPIN, HIGH);
    }   
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });
  
  server.on("/sprinkler", HTTP_GET, [](AsyncWebServerRequest *request){
    if(digitalRead(SPRINKLERPIN)){
    digitalWrite(SPRINKLERPIN, LOW);
    }
    else{
    digitalWrite(SPRINKLERPIN, HIGH);
    }   
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });

  // Start server
  server.begin();
}

/*==========================================================================================Main LOOP ================================
======================================================================================================================================*/

void loop(){
  
  controlUnitFunction();
  
  if(counter==60){
    sendDataToDatabase();
    sendSettingsToDatabase();
  
    counter=0;
  }
  if(counter%10==0) reciveStatusDataFromDatabse();
  if(counter%5==0) tempMeasurement();
  
  counter++;
  delay(10000);  
}
