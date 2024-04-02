#ifdef ARDUINO_ARCH_ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#else
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>


#define CS_USE_LITTLEFS true


#include <FS.h>
#include <Arduino.h>

#include <LittleFS.h>
#include <ArduinoJson.h>


#define PVERSION 3
#define FORMAT_LITTLEFS_IF_FAILED false

 String ssid;
 String pass;
 String ip;
 String gateway;
int wifi_state;
int PrefStatus;

AsyncWebServer manserver(80);

//Variables to save values from HTML form



// Search for parameter in HTTP POST request
const char* PARAM_INPUT_1 = "ssid";
const char* PARAM_INPUT_2 = "pass";
const char* PARAM_INPUT_3 = "ip";
const char* PARAM_INPUT_4 = "gateway";

void setup_SoftAP();
void setup_Application();
JsonDocument configDoc;

// Loads the configuration from a file
int loadJson(const char *filename) {
  
  // Open file for reading
  File thefile = LittleFS.open(filename,FILE_READ);

  // Deserialize the JSON document
  DeserializationError error = deserializeJson(configDoc, thefile);
  if (error){
    thefile.close();
    Serial.println(F("Failed to read file\n"));
    return(-1);
    }
  thefile.close();
  return(0);
}    

void saveJson(const char *filename) {
  // Open file for reading
  File file = LittleFS.open(filename,FILE_WRITE);

  // Deserialize the JSON document
  if (serializeJson(configDoc, file) == 0){
    Serial.println(F("Failed to write file\n"));
    }
  file.close();
}    

void save_config()
{
  //JsonDocument configDoc;

    configDoc["ssid"] = ssid;
    configDoc["pass"] = pass;
    configDoc["ip"] = ip;
    configDoc["gateway"] = gateway;
    configDoc["wifi_state"] = wifi_state;
    configDoc["version"] = PrefStatus;

   saveJson("/config.json");
}





void read_config()
{


    int error = loadJson("/config.json");
    serializeJsonPretty(configDoc, Serial);
    int the_version = configDoc["version"]; PrefStatus = the_version;
    if (PrefStatus != PVERSION){
      Serial.println("Warning: no config");
      PrefStatus = PVERSION;    
      ssid = "";
      pass = "";
      ip = "";
      gateway = "";
      wifi_state = 1;
      saveJson("/config.json");
      return;
      }
    

    String the_ssid = configDoc["ssid"]; ssid = the_ssid;
    String the_pass = configDoc["pass"]; pass = the_pass;
    String the_ip   = configDoc["ip"]; ip = the_ip;
    String the_gate = configDoc["gateway"]; gateway = the_gate;
    int the_wifi_state = configDoc["wifi_state"]; wifi_state = the_wifi_state;
    

}
void setup_wifi_manager()
{

  Serial.println("WifiManager: Starting Up\n");
  Serial.flush();
  Serial.println("wifimanager: LittleFS Mount in progress");
  Serial.flush();
  if(!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED)){
        Serial.println("LittleFS Mount Failed");
        return;
    };

  Serial.println("wifimanager: Preferences being configured");
  Serial.flush();
  
  read_config();
   
  Serial.print("wifimanager: Preference State: ");
  Serial.println(PrefStatus);
  Serial.print("wifi manager: Wifi State: ");
  Serial.println(wifi_state);
  Serial.flush();
  

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
           //setup_Application();     
    }          

}

void setup_SoftAP()
{
String macaddress;
char APname[32];

// Connect to Wi-Fi network with SSID and password
    Serial.println("Setting Up SoftAP (Access Point)");
   
    WiFi.softAP("blinds",NULL);

    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP); 
    Serial.println("Waiting on User to configure me");
      
 // Web Server Root URL
    manserver.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      Serial.println("Request slash");
      request->send(LittleFS, "/wifimanager.html", "text/html");
       });
    
    manserver.serveStatic("/", LittleFS, "/");
    
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
            save_config();
          }
          // HTTP POST pass value
          if (p->name() == PARAM_INPUT_2) {
            pass = p->value().c_str();
            Serial.print("Password set to: ");
            Serial.println(pass);
            // Write file to save value
            save_config();
          }
          // HTTP POST ip value
          if (p->name() == PARAM_INPUT_3) {
            ip = p->value().c_str();
            Serial.print("IP Address set to: ");
            Serial.println(ip);
            // Write file to save value
            save_config();
          }
          // HTTP POST gateway value
          if (p->name() == PARAM_INPUT_4) {
            gateway = p->value().c_str();
            Serial.print("Gateway set to: ");
            Serial.println(gateway);
            // Write file to save value
            save_config();
          }
          //Serial.printf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
        }
      }
    //preferences.end();
    request->send(200, "text/plain", "Done. OpenBlinds will restart, connect to your router and go to IP address: " + ip);
      wifi_state = 2;
      save_config();
      delay(3000);
      ESP.restart();
    });
    manserver.begin();
  
}

boolean connectWifi(){
  boolean state = true;
  int i = 0;
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  Serial.println("");
  Serial.println("Connecting to WiFi");

  // Wait for connection
  Serial.print("Connecting...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (i > 20){
      state = false; break;
    }
    i++;
  }
  Serial.println("");
  if (state){
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }
  else {
    Serial.println("Connection failed.");
  }
  delay(100);
  return state;
}

