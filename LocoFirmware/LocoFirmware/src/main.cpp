#include <Arduino.h>
#include <iostream>
#include <string>
#include <INA219.h>
#include <TinyGPSPlus.h>
#include <WiFi.h> 
#include <PubSubClient.h>
#include "Wire.h"

INA219 INA(0x40);

#define RXp2 16
#define TXp2 17

// Motor
int motorPin = 27; 
int pwmPin = 14; 

// Setting PWM properties
const int freq = 8000;
const int pwmChannel = 0;
const int resolution = 8;

/*Wifi*/
// Replace the next variables with your SSID/Password combination
const char* ssid = "DemoIoT";
const char* password = "DemoIoT1";
/*MQTT broker*/
//MQTT Broker IP address:
const char* mqtt_server = "49.12.32.132";
static bool MotorState = false;


WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;


void displayInfo();
void sendGeo();
void setup_wifi();
void reconnect();
void callback(char* topic, byte* message, unsigned int length);

// The TinyGPSPlus object
TinyGPSPlus gps;

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RXp2, TXp2);

  if (!INA.begin() )
  {
    Serial.println("could not connect. Fix and Reboot");
  }
  INA.setMaxCurrentShunt(2.5, 0.002);

  setup_wifi();

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  //Motor
  pinMode(motorPin, OUTPUT);
  pinMode(pwmPin, OUTPUT);

  // configure LED PWM functionalitites
  ledcSetup(pwmChannel, freq, resolution);
  
  // attach the channel to the GPIO to be controlled
  ledcAttachPin(pwmPin, pwmChannel);
}

// For stats that happen every 5 seconds
unsigned long last = 0UL;
static uint32_t dutyCycle = 0;
void loop() {
  if (MotorState == true)
  {
    digitalWrite(motorPin, LOW);
    ledcWrite(pwmChannel, dutyCycle); 
  }
  else
  {
    digitalWrite(motorPin, LOW);
    ledcWrite(pwmChannel, 0); 
  }
  // This sketch displays information every time a new sentence is correctly encoded.
  while (Serial2.available() > 0)
  {
    if (gps.encode(Serial2.read()))
    {
      displayInfo();
      sendGeo();
    }
  }

  if (millis() > 5000 && gps.charsProcessed() < 10)
  {
    Serial.println(F("No GPS detected: check wiring."));
    while(true);
  }
    if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 1000) {
    lastMsg = now;
    
    /*Ina sensor*/
    float voltage = INA.getBusVoltage();
    float current = INA.getCurrent_mA();
    float power = INA.getPower_mW();
    // Convert the value to a char array
    char tempString[8];
    dtostrf(voltage, 1, 2, tempString);
    client.publish("loco/voltage", tempString);
    dtostrf(current, 1, 2, tempString);
    client.publish("loco/current", tempString);
    dtostrf(power, 1, 2, tempString);
    client.publish("loco/power", tempString);

    Serial.print(F("Voltage: "));
    Serial.print(voltage);
    Serial.println();
    Serial.print(F("Current: "));
    Serial.print(current);
    Serial.println();
    Serial.print(F("Power: "));
    Serial.print(power);
    Serial.println();
  }
}

void displayInfo()
{
  Serial.print(F("Location: ")); 
  if (gps.location.isValid())
  {
    Serial.print(gps.location.lat(), 6);
    Serial.print(F(","));
    Serial.print(gps.location.lng(), 6);
  }
  else
  {
    Serial.print(F("INVALID"));
  }

  Serial.print(F("  Date/Time: "));
  if (gps.date.isValid())
  {
    Serial.print(gps.date.month());
    Serial.print(F("/"));
    Serial.print(gps.date.day());
    Serial.print(F("/"));
    Serial.print(gps.date.year());
  }
  else
  {
    Serial.print(F("INVALID"));
  }

  Serial.print(F(" "));
  if (gps.time.isValid())
  {
    if (gps.time.hour() < 10) Serial.print(F("0"));
    Serial.print(gps.time.hour());
    Serial.print(F(":"));
    if (gps.time.minute() < 10) Serial.print(F("0"));
    Serial.print(gps.time.minute());
    Serial.print(F(":"));
    if (gps.time.second() < 10) Serial.print(F("0"));
    Serial.print(gps.time.second());
    Serial.print(F("."));
    if (gps.time.centisecond() < 10) Serial.print(F("0"));
    Serial.print(gps.time.centisecond());
  }
  else
  {
    Serial.print(F("INVALID"));
  }

  Serial.println();
}

void sendGeo()
{
  if (gps.location.isValid())
  {
    char tempStringLat[9] = {0};
    double lat = gps.location.lat();
    dtostrf(lat, 1, 6, tempStringLat);
    client.publish("loco/location/lat", tempStringLat);
    Serial.print(tempStringLat);

    char tempStringLng[9] = {0};
    double lng = gps.location.lng();
    dtostrf(lng, 1, 6, tempStringLng);
    client.publish("loco/location/lng", tempStringLng);
  }
  else
  {
    //Do nothing. We do not want to transmit invalid values;
  }

  if ((gps.date.isValid()) && (gps.time.isValid()))
  {
    struct tm t = {0};
    time_t epoch;

    t.tm_year = (gps.date.year()-1900);
    t.tm_mon =  (gps.date.month() - 1);
    t.tm_mday = gps.date.day();
    t.tm_hour = gps.time.hour();
    t.tm_min = gps.time.minute();
    t.tm_sec = gps.time.second();

    epoch = mktime(&t);
    char tempStringTime[sizeof(epoch)];
    dtostrf(epoch, 1, 0, tempStringTime);
    client.publish("loco/location/timestamp", tempStringTime);
  }
  else
  {
      //Do nothing. We do not want to transmit invalid values;
  }

}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  delay(500);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();
  // Changes the output state according to the message
  if (String(topic) == "loco/control/lights/switch") {
    Serial.print("Changing lights to ");
    if(messageTemp == "on"){
      Serial.println("on");
      //digitalWrite(ledPin, HIGH);
    }
    else if(messageTemp == "off"){
      Serial.println("off");
      //digitalWrite(ledPin, LOW);
    }
  }
  if (String(topic) == "loco/control/motor/switch") {
    Serial.print("Changing motor state to ");
    if(messageTemp == "on"){
      Serial.println("on");
      MotorState = true;
    }
    else if(messageTemp == "off"){
      Serial.println("off");
      MotorState = false;
    }
  }
  if (String(topic) == "loco/control/motor/power") {
    MotorState = true;
    dutyCycle = messageTemp.toInt();
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("LocoClient")) {
      Serial.println("connected");
      // Subscribe
      client.subscribe("loco/control/motor/power");
      client.subscribe("loco/control/motor/switch");
      client.subscribe("loco/control/lights/switch");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
