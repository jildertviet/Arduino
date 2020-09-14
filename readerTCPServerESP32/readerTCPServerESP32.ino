#include <WiFi.h>
#include <ArduinoOTA.h>
#include "EEPROM.h"

#include "CRC16.h"
#define MAX_CLIENTS 5

const char* ssid = "";
const char* password = "";

bool bSend = true;

//EEPROMClass  startValuesPerDayEeprom("startValuesPerDay", 0x300);
long startValuesPerDay[31][3];
String eepromString = "";
WiFiServer server(8888);

const int led = 5;

long mEVLT = 0; //Meter reading Electrics - consumption low tariff
long mEVHT = 0; //Meter reading Electrics - consumption high tariff
long mEOLT = 0; //Meter reading Electrics - return low tariff
long mEOHT = 0; //Meter reading Electrics - return high tariff
long mEAV = 0;  //Meter reading Electrics - Actual consumption
long mEAT = 0;  //Meter reading Electrics - Actual return
long mGAS = 0;    //Meter reading Gas
long prevGAS = 0;
String timeString = "";
unsigned char msgDate[3] = {0, 0, 0};
unsigned char msgTime[3] = {0, 0, 0}; // H, m, s
unsigned char currentDay = 255;

unsigned int currentCRC = 0;
//unsigned char allRecords[86400][2];
//int writePos = 0; 

#define MAXLINELENGTH 256 // longest normal line is 47 char (+3 for \r\n\0)
char telegram[MAXLINELENGTH];

#define SERIAL_RX     33  // pin for SoftwareSerial RX

bool checkDayTransition(){
  if(currentDay == 255){
    currentDay = msgDate[2];
    return false;
  }
  if(msgDate[2] != currentDay){
    currentDay = msgDate[2];
    return true;
  } else{
    currentDay = msgDate[2];
    return false;
  }
}

void shortBlink(int num = 1, int delayTime=200) {
  for (int i = 0; i < num; i++) {
    delay(delayTime);
    digitalWrite(led, LOW);
    delay(delayTime);
    digitalWrite(led, HIGH);
  }
}

bool isNumber(char* res, int len) {
  for (int i = 0; i < len; i++) {
    if (((res[i] < '0') || (res[i] > '9'))  && (res[i] != '.' && res[i] != 0)) {
      return false;
    }
  }
  return true;
}

int FindCharInArrayRev(char array[], char c, int len) {
  for (int i = len - 1; i >= 0; i--) {
    if (array[i] == c) {
      return i;
    }
  }
  return -1;
}

long getValidVal(long valNew, long valOld, long maxDiffer) {
  //check if the incoming value is valid
  if (valOld > 0 && ((valNew - valOld > maxDiffer) && (valOld - valNew > maxDiffer)))
    return valOld;
  return valNew;
}

long getValue(char* buffer, int maxlen) {
  int s = FindCharInArrayRev(buffer, '(', maxlen - 2);
  if (s < 8) return 0;
  if (s > 32) s = 32;
  int l = FindCharInArrayRev(buffer, '*', maxlen - 2) - s - 1;
  if (l < 4) return 0;
  if (l > 12) return 0;
  char res[16];
  memset(res, 0, sizeof(res));

  if (strncpy(res, buffer + s + 1, l)) {
    if (isNumber(res, l)) {
      return (1000 * atof(res));
    }
  }
  return 0;
}

bool decodeTelegram(int len) {
  //need to check for start
  int startChar = FindCharInArrayRev(telegram, '/', len);
  int endChar = FindCharInArrayRev(telegram, '!', len);
  bool validCRCFound = false;
  if (startChar >= 0){
    //start found. Reset CRC calculation
    currentCRC = CRC16(0x0000, (unsigned char *) telegram + startChar, len - startChar);
  }
  else if (endChar >= 0){
    //add to crc calc
    currentCRC = CRC16(currentCRC, (unsigned char*)telegram + endChar, 1);
    char messageCRC[5];
    strncpy(messageCRC, telegram + endChar + 1, 4);
    messageCRC[4] = 0; //thanks to HarmOtten (issue 5)
    validCRCFound = (strtol(messageCRC, NULL, 16) == currentCRC);
    if (validCRCFound)
      Serial.println("\nVALID CRC FOUND!");
    else
      Serial.println("\n===INVALID CRC FOUND!===");
    currentCRC = 0;
  }
  else{
    currentCRC = CRC16(currentCRC, (unsigned char*)telegram, len);
  }

  long val = 0;
  long val2 = 0;

  if(strncmp(telegram, "0-0:1.0.0", strlen("0-0:1.0.0")) == 0){
    timeString = String(telegram);
//    msgDate[0] = timeString.substring(strlen("0-0:1.0.0(")+(2*0), strlen("0-0:1.0.0(")+(2*0)+2).toInt(); // Year
//    msgDate[1] = timeString.substring(strlen("0-0:1.0.0(")+(2*1), strlen("0-0:1.0.0(")+(2*1)+2).toInt(); // Month
    msgDate[2] = timeString.substring(strlen("0-0:1.0.0(")+(2*2), strlen("0-0:1.0.0(")+(2*2)+2).toInt(); // Day
    msgTime[0] = timeString.substring(strlen("0-0:1.0.0(")+(2*3), strlen("0-0:1.0.0(")+(2*3)+2).toInt(); // Hour
    msgTime[1] = timeString.substring(strlen("0-0:1.0.0(")+(2*4), strlen("0-0:1.0.0(")+(2*4)+2).toInt(); // Minute
    msgTime[2] = timeString.substring(strlen("0-0:1.0.0(")+(2*5), strlen("0-0:1.0.0(")+(2*5)+2).toInt(); // Second
    if(millis() > 8000){
      if(checkDayTransition()){ // NEW DAY
        char longArray[3][8];
        memcpy(longArray[0], &mGAS, 8);
        long sum = mEVLT + mEVHT;
        memcpy(longArray[1], &sum, 8);
        sum = mEOLT + mEOHT;
        memcpy(longArray[2], &sum, 8);
        for(char j=0; j<8; j++){
          EEPROM.write(msgDate[2]*8*3 + (8*0) + j, longArray[0][j]);
          EEPROM.write(msgDate[2]*8*3 + (8*1) + j, longArray[1][j]);
          EEPROM.write(msgDate[2]*8*3 + (8*2) + j, longArray[2][j]);
        }
        EEPROM.commit();
        
        startValuesPerDay[msgDate[2]][0] = mGAS;
        startValuesPerDay[msgDate[2]][1] = mEVLT + mEVHT;
        startValuesPerDay[msgDate[2]][2] = mEOLT + mEOHT;      
      }
    }
  }

  // 1-0:1.8.1(000992.992*kWh)
  // 1-0:1.8.1 = Elektra verbruik laag tarief (DSMR v4.0)
  if (strncmp(telegram, "1-0:1.8.1", strlen("1-0:1.8.1")) == 0)
    mEVLT =  getValue(telegram, len);
  
  // 1-0:1.8.2(000560.157*kWh)
  // 1-0:1.8.2 = Elektra verbruik hoog tarief (DSMR v4.0)
  if (strncmp(telegram, "1-0:1.8.2", strlen("1-0:1.8.2")) == 0){
    mEVHT = getValue(telegram, len);
  }


  // 1-0:2.8.1(000348.890*kWh)
  // 1-0:2.8.1 = Elektra opbrengst laag tarief (DSMR v4.0)
  if (strncmp(telegram, "1-0:2.8.1", strlen("1-0:2.8.1")) == 0){
    mEOLT = getValue(telegram, len);
  }


  // 1-0:2.8.2(000859.885*kWh)
  // 1-0:2.8.2 = Elektra opbrengst hoog tarief (DSMR v4.0)
  if (strncmp(telegram, "1-0:2.8.2", strlen("1-0:2.8.2")) == 0)
    mEOHT = getValue(telegram, len);


  // 1-0:1.7.0(00.424*kW) Actueel verbruik
  // 1-0:2.7.0(00.000*kW) Actuele teruglevering
  // 1-0:1.7.x = Electricity consumption actual usage (DSMR v4.0)
  if (strncmp(telegram, "1-0:1.7.0", strlen("1-0:1.7.0")) == 0) {
    mEAV = getValue(telegram, len);
    //    Serial.println("mEAV FOUND!!!: "); Serial.println(mEAV);
  }

  if (strncmp(telegram, "1-0:2.7.0", strlen("1-0:2.7.0")) == 0)
    mEAT = getValue(telegram, len);


  // 0-1:24.2.1(150531200000S)(00811.923*m3)
  // 0-1:24.2.1 = Gas (DSMR v4.0) on Kaifa MA105 meter
  if (strncmp(telegram, "0-1:24.2.1", strlen("0-1:24.2.1")) == 0){
    mGAS = getValue(telegram, len);
  }

  return validCRCFound;
  //  return true;
}

bool readTelegram() {
  if (Serial1.available()) {
    memset(telegram, 0, sizeof(telegram));
    while (Serial1.available()) {
      int len = Serial1.readBytesUntil('\n', telegram, MAXLINELENGTH);
      telegram[len] = '\n';
      telegram[len + 1] = 0;
      yield();
      if (decodeTelegram(len + 1)){
//        allRecords[writePos][0] = (mEAV/3000.0)*255;
//        allRecords[writePos][1] = (mEAT/3000.0)*255;
//        writePos++;
//        shortBlink(1, 30);
        return true;
      } else{
        return false;
      }
    }
  } else {
    delay(1);
    return false;
  }
}

void initOta(){
  ArduinoOTA.setHostname("slimmeMeterESP32");
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
}

void setup(void) {
  pinMode(led, OUTPUT);
  digitalWrite(led, HIGH);

  Serial.begin(115200);
  Serial1.begin(115200, SERIAL_8N1, 33, -1, true);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println();

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if(millis() > 5000)
      ESP.restart();
  }
  Serial.print("\nConnected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  initOta();
  server.begin();

  EEPROM.begin(768);
//  for(int i=0; i<
  char eepromBuffer[768];
  for(int i=0; i<768; i++)
    eepromBuffer[i] = EEPROM.read(i);
  
  for(char i=0; i<31; i++){
//    eepromString = eepromString + "[" + String((int)i) + "]: ";
    for(char j=0; j<3; j++){
      memcpy(&startValuesPerDay[i][j], eepromBuffer + (i*8*3 + (8*j)), 8);
//      eepromString = eepromString + String(startValuesPerDay[i][j]) + " ";
    }
//    eepromString = eepromString + "\n";
  }
  
  shortBlink(2);
}

void loop(void) {
  ArduinoOTA.handle();
  readTelegram();
  
  // Check for new connections
  WiFiClient client = server.available();
  if(client){
    String msg = "\nTime:\t\t" + String((int)msgTime[0]) + ":" + String((int)msgTime[1]) + ":" + String((int)msgTime[2]) + "\n" + 
    "Verbruik:\t" + String(mEAV) + "W\n" + 
    "Leveren:\t" + String(mEAT) + "W\n" + 
    "Verbruikt:\t" + String((mEVLT + mEVHT)/1000.) + "kWh\n" + 
    "Teruggeleverd:\t" + String((mEOLT + mEOHT)/1000.) + "kWh\n" + 
    "Gas:\t\t" + String(mGAS/1000.) + "m3\n" + 
    "\nVandaag:\n" + 
    "Verbruikt:\t" + String(((mEVLT + mEVHT) - startValuesPerDay[msgDate[2]][1])/1000.) + "kWh\n" + 
    "Teruggeleverd:\t" + String(((mEOLT + mEOHT) - startValuesPerDay[msgDate[2]][2])/1000.) + "kWh\n" + 
    "Gas:\t\t" + String((mGAS - startValuesPerDay[msgDate[2]][0])/1000.) + "m3\n\n";

//    msg = eepromString;
    client.write(msg.c_str());
    client.stop();
  }
}
