/*
 * This is an example on how to use Espalexa alongside an ESPAsyncWebServer.
 */
 
#define ESPALEXA_ASYNC //it is important to define this before #include <Espalexa.h>!
#include <Espalexa.h>

#ifdef ARDUINO_ARCH_ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#else
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>
#include "SPIFFS.h"
#include <Preferences.h>
#include "globals.h"

// prototypes
boolean connectWifi();
AsyncWebServer myserver(80);

String ssid;
String pass;
String ip;
String gateway;
uint wifi_state;


IPAddress localIP;
//IPAddress localIP(192, 168, 1, 200); // hardcoded

// Set your Gateway IP address
IPAddress localGateway;
//IPAddress localGateway(192, 168, 1, 1); //hardcoded
IPAddress subnet(255, 255, 0, 0);


//callback functions
void firstLightChanged(uint8_t brightness);

void setup_wifi_manager();

boolean wifiConnected = false;

Espalexa espalexa;


void setup_Application()
{
wifiConnected = connectWifi();
  
  if(wifiConnected){
    myserver.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(200, "text/plain", "This is an example index page your server may send.");
    });
    myserver.on("/test", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(200, "text/plain", "This is a second subpage you may have.");
    });
    myserver.onNotFound([](AsyncWebServerRequest *request){
      if (!espalexa.handleAlexaApiCall(request)) //if you don't know the URI, ask espalexa whether it is an Alexa control request
      {
        //whatever you want to do with 404s
        request->send(404, "text/plain", "Not found");
      }
    });

    // Define your devices here.
    espalexa.addDevice("My Light 1", firstLightChanged); //simplest definition, default state off

    espalexa.begin(&myserver); //give espalexa a pointer to your server object so it can use your server instead of creating its own
    //server.begin(); //omit this since it will be done by espalexa.begin(&server)
  } else
  {
    while (1)
    {
      Serial.println("Cannot connect to WiFi. Please check data and reset the ESP.");
      delay(2500);
    }
  }
}
 
void loop()
{
   //espalexa.loop();
   delay(1);
}

//our callback functions
void firstLightChanged(uint8_t brightness) {
    Serial.print("Device 1 changed to ");
    
    //do what you need to do here

    //EXAMPLE
    if (brightness == 255) {
      Serial.println("ON");
    }
    else if (brightness == 0) {
      Serial.println("OFF");
    }
    else {
      Serial.print("DIM "); Serial.println(brightness);
    }
}



void setup()
{
  Serial.begin(115200);
  // Initialise wifi connection
  Serial.println("Booting");
  Serial.flush();
  delay(1);
  setup_wifi_manager();
}
   
// connect to wifi – returns true if successful or false if not
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

