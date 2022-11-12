#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>

static int  webserver_dcspeed = 0;
static int  webserver_dcdirection = 0;
static bool webserver_dcswitch = false; 
static int  webserver_power = 0;

static void setup_routing();

// Web server running on port 80
WebServer server(80);
 
// JSON data buffer
StaticJsonDocument<250> jsonDocument;
char buffer[250];

void webserver_setup() {
  setup_routing(); 	 	 
}	 	 
  	 	 
void webserver_update() {	 	 
  server.handleClient();	 	 
}

void webserver_setPower(int power)
{ 
  webserver_power = power;
}

void webserver_getMotor(int *dcspeed, int *dcdirection, bool *dcswitch)
{
  *dcspeed = webserver_dcspeed;
  *dcdirection = webserver_dcdirection;
  *dcswitch = webserver_dcswitch;
}
/* private functions*/
static void setup_routing() {	 	  
  server.on("/power", getPower);	 	 
  server.on("/motor", HTTP_POST, handlePost);	 	 
  // start server	 	 
  server.begin();	 	 
}

static void create_json(char *tag, float value, char *unit) {  
  jsonDocument.clear();  
  jsonDocument["type"] = tag;
  jsonDocument["value"] = value;
  jsonDocument["unit"] = unit;
  serializeJson(jsonDocument, buffer);
}
 
static void add_json_object(char *tag, float value, char *unit) {
  JsonObject obj = jsonDocument.createNestedObject();
  obj["type"] = tag;
  obj["value"] = value;
  obj["unit"] = unit; 
}
 
static void getPower() {
  Serial.println("Get power");
  create_json("power", power, "mW");
  server.send(200, "application/json", buffer);
}

static void handlePost() {
  if (server.hasArg("plain") == false) {
    //handle error here
  }
  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);
  
  // Get RGB components
  int webserver_dcspeed = jsonDocument["speed"];
  int webserver_dcdirection = jsonDocument["direction"];
  int webserver_dcswitch = jsonDocument["switch"];

  // Respond to the client
  server.send(200, "application/json", "{}");
}