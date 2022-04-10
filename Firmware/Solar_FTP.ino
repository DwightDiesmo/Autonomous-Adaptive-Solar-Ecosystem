#include <WiFi.h>
#include <BSP.h>
#include <SDMMCBlockDevice.h>
#include "FATFileSystem.h"

#define FTP_PORT 21 //FTP port number, usually 21 or 22 default

#define FTP_USERNAME "solarOptimize" //username to access FTP server
#define FTP_PASSWORD "sunny123" //password to access FTP server

#define FATFileSystem_NAME "fs" //fs (filesystem) while using files

#define PASVPORT 50001

#define WIFI_IP1 192 //WiFi IP address
#define WIFI_IP2 168 //WiFi IP address
#define WIFI_IP3 43 //WiFi IP address
#define WIFI_IP4 132 //WiFi IP address

char outBuf[128]; //buffer of 128 char for FTP server to recieve
char outCount; //buffer counter

SDMMCBlockDevice block_device;
mbed::FATFileSystem fs(FATFileSystem_NAME);
FILE *myFile;

IPAddress server(WIFI_IP1, WIFI_IP2, WIFI_IP3, WIFI_IP4);

WiFiClient client;
WiFiClient dclient;


void mountSD();
void setupWiFi();
byte receiveBytes();
int FTPConnect();


void setup() {
  Serial.begin(921600);
  mountSD();
  setupWiFi();  
}

void loop() {
  myFile = fopen("fs/ecosystem.txt", "wb+");
  fprintf(myFile, "123");
  fclose(myFile);
  FTPConnect();
  delay(500);
}

int FTPConnect() {
  //open and prep certain file to send (function)
  bool FTPRun = 1;
  while(FTPRun){
    if(client.connect(server, FTP_PORT)) {
      Serial.println("Connected"); //TEST LINE REMOVE
    }
    else {
      Serial.println("Failed"); //TEST LINE REMOVE
    }
    Serial.print("FILE: "); Serial.println("ecosystem.txt");
    myFile = fopen("fs/ecosystem.txt", "rb");
    if(myFile == NULL) {
      Serial.println("SD OPEN FAIL");
      return 0;
    }
  
    if(!receiveBytes()) return 0; //constantly try to get recieveBytes() to pass and return 1
    client.print(F("USER ")); client.println(F(FTP_USERNAME)); //login username for FTP server
    Serial.print(F("USER ")); Serial.println(F(FTP_USERNAME));
    if(!receiveBytes()) return 0;
    client.print(F("PASS ")); client.println(F(FTP_PASSWORD)); //login password for FTP server
    Serial.print(F("PASS ")); Serial.println(F(FTP_PASSWORD));
    if(!receiveBytes()) return 0;
    client.println(F("SYST"));
    Serial.println(F("SYST")); 
    if(!receiveBytes()) return 0;
    client.println(F("PASV"));
    Serial.println(F("PASV")); 
    if(!receiveBytes()) return 0;
    Serial.print(F("Data port: "));
    Serial.println(PASVPORT);

    if (dclient.connect(server,PASVPORT)) {
      Serial.println(F("Data connected"));
    } 
    else {
      Serial.println(F("Data connection failed"));
      client.stop();
      fclose(myFile);
      return 0;
    }
    client.println(F("CWD /"));
    client.print(F("STOR ")); Serial.print("STOR ");
    client.println("ecosystem.txt"); Serial.println("ecosystem.txt");
    if(!receiveBytes()) return 0; Serial.println("Reading and Writing");
    int sz = 0;
    char buffer1[128]; //64
    //explicit_bzero(buffer1, 128); //64
    bzero(buffer1, 128); ////////////////////////////////////////////////////////////
    fseek(myFile, 0, SEEK_END);
    sz = ftell(myFile);
    rewind(myFile);
    Serial.println(sz);
    while(1)
    {
      if(feof(myFile))
      {
        break;
      }
      if(sz > 127) //63
      {
        fgets(buffer1, 128, myFile); //64
        dclient.write(buffer1, 127); //63
        sz = sz - 127; //63
        //Serial.println("sz > 63");
        delay(1);
      }
      if(sz > 0 && sz < 128) //64
      {
        fgets(buffer1, 128, myFile); //64
        dclient.write(buffer1, sz);
        //Serial.println("sz > 0 && sz < 64");
        delay(1);
      }   
    }
    dclient.stop();
    fclose(myFile);
    //remove("test.txt"); /////////////////////////REMOVE LINE///////////////////////////////////
    Serial.println(F("Data disconnected"));
    client.println("QUIT");
    if(!receiveBytes()) return 0;
    client.stop();
    Serial.println(F("Command disconnected"));
    FTPRun = 0;
  }
  return 1;
}

byte receiveBytes() {
  byte response; //to alert if there is an emergency
  byte readByte; //read bytes 128 at a time
  while(!client.available()) delay(1); //constantly try to see if client is available
  response = client.peek(); //read byte from file and NOT advance to next
  outCount = 0; //set initial counter to zero
  while(client.available()) {
    readByte = client.read(); //read byte from server
    Serial.write(readByte); 
    if(outCount < 127) {
      outBuf[outCount] = readByte; //load the read byte into buffer that will store 128 bytes at a time
      outCount++;
      outBuf[outCount] = 0; //then put next count with a temporary zero in the buffer
    }
  }
  if(response >= '4') {//trigger emergency and QUIT connection
    readByte = 0; //initially set to zero of read byte
    client.println(F("QUIT")); //QUIT statement for FTP
    while(!client.available()) delay(1); //constantly try to see if client is available
    while(client.available()) {
      readByte = client.read(); //load latest byte from server to client
      Serial.write(readByte);
    }
    client.stop(); //stop client connection to server
    Serial.println("Disconnected"); //TEST LINE REMOVE
    Serial.println("SD closed"); //TEST LINE REMOVE
    return 0; //this fails and will then request to recall recieveBytes()
  }
  return 1; //this passes and allows for client.print() in FTP server, want to keep state bitstream
}

void mountSD() {
  int err = fs.mount(&block_device); // return one if 1 if could not mount SD
    if (err) {
    // Reformat if we can't mount the filesystem
    // this should only happen on the first boot
    Serial.println("No filesystem found, please check on computer and manually format");
    err = fs.reformat(&block_device);  // seriously don't want to format good data
  }
  if (err) {
     Serial.println("Error formatting SDCARD ");
     while(1);
  }
}


void setupWiFi() {
  char ssid[] = "OnePlus 5T"; //network SSID (name) //80FB54 //rileyDesktop
  char pass[] = "riley123"; //network password //W2T76B2B01189 //teamargus2
  int status = WL_IDLE_STATUS; //WiFi radio status;

  //attempt to connect to WiFi network:
  while(status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: "); //TEST LINE REMOVE
    Serial.println(ssid); //TEST LINE REMOVE
    status = WiFi.begin(ssid, pass); //Connect to exclusively WPA/WPA2 network

  }
  Serial.println("Connected to wifi"); //TEST LINE REMOVE 
}
