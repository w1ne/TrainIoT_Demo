#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>

// Web server running on port 80
WebServer server(80);
 
// JSON data buffer
StaticJsonDocument<250> jsonDocument;
char buffer[250];

void setup_routing() {	 	  
  server.on("/power", getPower);	 	 
  server.on("/led", HTTP_POST, handlePost);	 	 
  // start server	 	 
  server.begin();	 	 
}
 
void create_json(char *tag, float value, char *unit) {  
  jsonDocument.clear();  
  jsonDocument["type"] = tag;
  jsonDocument["value"] = value;
  jsonDocument["unit"] = unit;
  serializeJson(jsonDocument, buffer);
}
 
void add_json_object(char *tag, float value, char *unit) {
  JsonObject obj = jsonDocument.createNestedObject();
  obj["type"] = tag;
  obj["value"] = value;
  obj["unit"] = unit; 
}
 
void getPower() {
  Serial.println("Get power");
  create_json("power", temperature, "mW");
  server.send(200, "application/json", buffer);
}
 
/*void getEnv() {
  Serial.println("Get env");
  jsonDocument.clear();
  add_json_object("temperature", temperature, "°C");
  add_json_object("humidity", humidity, "%");
  add_json_object("pressure", pressure, "mBar");
  serializeJson(jsonDocument, buffer);
  server.send(200, "application/json", buffer);
}
*/

void setup_task() {	 	 
  xTaskCreate(	 	 
  read_sensor_data, 	 	 
  "Read sensor data", 	 	 
  1000, 	 	 
  NULL, 	 	 
  1, 	 	 
  NULL 	 	 
  );	 	 
}
void setup() {	 	 
  connectToWiFi();	 	 
  setup_task();	 	 
  setup_routing(); 	 	 
  // Initialize Neopixel	 	 
  pixels.begin();	 	 
}	 	 
  	 	 
void loop() {	 	 
  server.handleClient();	 	 
}