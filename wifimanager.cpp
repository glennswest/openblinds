
#include <Preferences.h>
#include "SPIFFS.h"
#ifdef ARDUINO_ARCH_ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#else
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>
#include "globals.h"

#define PVERSION 2

void initSPIFFS();
AsyncWebServer manserver(80);

//Variables to save values from HTML form

Preferences preferences;

// Search for parameter in HTTP POST request
const char* PARAM_INPUT_1 = "ssid";
const char* PARAM_INPUT_2 = "pass";
const char* PARAM_INPUT_3 = "ip";
const char* PARAM_INPUT_4 = "gateway";

void setup_SoftAP();
void setup_Application();

void setup_wifi_manager()
{

  Serial.println("WifiManager: Starting Up\n");
  Serial.flush();
  Serial.println("wifimanager: initSPIFFS in progress");
  Serial.flush();
  initSPIFFS();
  Serial.println("wifimanager: Preferences being configured");
  Serial.flush();
  preferences.begin("wifimanager", false); 
  unsigned int PrefStatus = preferences.getUInt("PrefStatus", 0);
  Serial.print("wifimanager: Preference State: ");
  Serial.println(PrefStatus);
  Serial.flush();
  if (PrefStatus < PVERSION){
    Serial.println("Warning: Preferences Updated");
    preferences.clear();
    PrefStatus = PVERSION;
    
    preferences.putUInt("PrefStatus", PrefStatus);
    ssid = "";
    pass = "";
    ip = "";
    gateway = "";
    wifi_state = 1;
    preferences.putString("wifi_ssid", ssid);
    preferences.putString("wifi_key",pass);
    preferences.putString("wifi_ip", ip);
    preferences.putString("wifi_gateway", gateway);
    preferences.putUInt("wifi_state", wifi_state);
    preferences.end();
    preferences.begin("wifimanager", false); 
    } else {
      Serial.println("Preferences Set");
      wifi_state = preferences.getUInt("wifi_state", 0);
    }

    switch(wifi_state){
      case 0:
           Serial.println("WifiManager: Unknown State! Error! Please request GURU Support");
           break;
      case 1:
           Serial.println("WifiManager: Configuration Mode - Please attach to OpenBlind AP to configure");
           setup_SoftAP();
           break;
      case 2:
           Serial.println("WifiManager: Starting application");
           setup_Application();     
    }          

}

void setup_SoftAP()
{
String macaddress;
char APname[32];

// Connect to Wi-Fi network with SSID and password
    Serial.println("Setting AP (Access Point)");
    // NULL sets an open Access Point
   // macaddress = WiFi.macAddress();
   // Serial.print("Mac Address: ");
   // Serial.println(macaddress);

    //snprintf(APname, 32, "blinds-%llX", ESP.getEfuseMac());
    //snprintf(APname,24,"blinds");
    //Serial.print("AP Name ");
    //Serial.println(APname);
    //WiFi.softAP(APname, NULL);
    WiFi.softAP("blinds",NULL);

    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP); 
    Serial.println("Waiting on User to configure me");
      
 // Web Server Root URL
    manserver.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      Serial.println("Request slash");
      request->send(SPIFFS, "/wifimanager.html", "text/html");
       });
    
    manserver.serveStatic("/", SPIFFS, "/");
    
    manserver.on("/", HTTP_POST, [](AsyncWebServerRequest *request) {
      int params = request->params();
      for(int i=0;i<params;i++){
        AsyncWebParameter* p = request->getParam(i);
        if(p->isPost()){
          // HTTP POST ssid value
          if (p->name() == PARAM_INPUT_1) {
            ssid = p->value().c_str();
            Serial.print("SSID set to: ");
            Serial.println(ssid);
            // Write file to save value
            preferences.putString("wifi_ssid", ssid);
          }
          // HTTP POST pass value
          if (p->name() == PARAM_INPUT_2) {
            pass = p->value().c_str();
            Serial.print("Password set to: ");
            Serial.println(pass);
            // Write file to save value
            preferences.putString("wifi_key", pass);
          }
          // HTTP POST ip value
          if (p->name() == PARAM_INPUT_3) {
            ip = p->value().c_str();
            Serial.print("IP Address set to: ");
            Serial.println(ip);
            // Write file to save value
            preferences.putString("wifi_ip", ip.c_str());
          }
          // HTTP POST gateway value
          if (p->name() == PARAM_INPUT_4) {
            gateway = p->value().c_str();
            Serial.print("Gateway set to: ");
            Serial.println(gateway);
            // Write file to save value
            preferences.putString("wifi_gateway", gateway.c_str());
          }
          //Serial.printf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
        }
      }
    //preferences.end();
    request->send(200, "text/plain", "Done. ESP will restart, connect to your router and go to IP address: " + ip);
      delay(3000);
      //ESP.restart();
    });
    manserver.begin();
  
}

// Initialize SPIFFS
void initSPIFFS() {
  
  if (!SPIFFS.begin(true)) {
    Serial.println("An error has occurred while mounting SPIFFS");
  }
  if (SPIFFS.exists("/wifimanager.html")){
      Serial.println("SPIFFS mounted successfully");
   } else {
      Serial.println("SPIFFS: No FILES available!");
   }
}

// Read File from SPIFFS
String readFile(fs::FS &fs, const char * path){
  Serial.printf("Reading file: %s\r\n", path);

  File file = fs.open(path);
  if(!file || file.isDirectory()){
    Serial.println("- failed to open file for reading");
    return String();
  }
  
  String fileContent;
  while(file.available()){
    fileContent = file.readStringUntil('\n');
    break;     
  }
  return fileContent;
}

// Write file to SPIFFS
void writeFile(fs::FS &fs, const char * path, const char * message){
  Serial.printf("Writing file: %s\r\n", path);

  File file = fs.open(path, FILE_WRITE);
  if(!file){
    Serial.println("- failed to open file for writing");
    return;
  }
  if(file.print(message)){
    Serial.println("- file written");
  } else {
    Serial.println("- write failed");
  }
}
