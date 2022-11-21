#include <Arduino.h>
#include <iostream>
#include <string>
#include <INA219.h>
#include <TinyGPSPlus.h>
#include <WiFi.h> 
#include <PubSubClient.h>
#include "Wire.h"
#include <WebServer.h>
#include <LoRa.h>
#include "pins.h"
#include "motor.h"

/*
TODO refractor it and separate in different modules, too crowdy
*/

//#define SECURE_MQTT // Comment this line if you are not using MQTT over SSL

#ifdef SECURE_MQTT
#include "esp_tls.h"

// Let's Encrypt CA certificate. Change with the one you need
static const unsigned char DSTroot_CA[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDSjCCAjKgAwIBAgIQRK+wgNajJ7qJMDmGLvhAazANBgkqhkiG9w0BAQUFADA/
MSQwIgYDVQQKExtEaWdpdGFsIFNpZ25hdHVyZSBUcnVzdCBDby4xFzAVBgNVBAMT
DkRTVCBSb290IENBIFgzMB4XDTAwMDkzMDIxMTIxOVoXDTIxMDkzMDE0MDExNVow
PzEkMCIGA1UEChMbRGlnaXRhbCBTaWduYXR1cmUgVHJ1c3QgQ28uMRcwFQYDVQQD
Ew5EU1QgUm9vdCBDQSBYMzCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEB
AN+v6ZdQCINXtMxiZfaQguzH0yxrMMpb7NnDfcdAwRgUi+DoM3ZJKuM/IUmTrE4O
rz5Iy2Xu/NMhD2XSKtkyj4zl93ewEnu1lcCJo6m67XMuegwGMoOifooUMM0RoOEq
OLl5CjH9UL2AZd+3UWODyOKIYepLYYHsUmu5ouJLGiifSKOeDNoJjj4XLh7dIN9b
xiqKqy69cK3FCxolkHRyxXtqqzTWMIn/5WgTe1QLyNau7Fqckh49ZLOMxt+/yUFw
7BZy1SbsOFU5Q9D8/RhcQPGX69Wam40dutolucbY38EVAjqr2m7xPi71XAicPNaD
aeQQmxkqtilX4+U9m5/wAl0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNV
HQ8BAf8EBAMCAQYwHQYDVR0OBBYEFMSnsaR7LHH62+FLkHX/xBVghYkQMA0GCSqG
SIb3DQEBBQUAA4IBAQCjGiybFwBcqR7uKGY3Or+Dxz9LwwmglSBd49lZRNI+DT69
ikugdB/OEIKcdBodfpga3csTS7MgROSR6cz8faXbauX+5v3gTt23ADq1cEmv8uXr
AvHRAosZy5Q6XkjEGB5YGV8eAlrwDPGxrancWYaLbumR9YbK+rlmM6pZW87ipxZz
R8srzJmwN0jP41ZL9c8PDHIyh8bwRLtTcm1D9SZImlJnt1ir/md2cXjbDaJWFBM5
JDGFoqgCWjBH4d1QB7wCCZAA62RjYJsWvIjJEubSfZGL+T0yjWW06XyxV3bqxbYo
Ob8VZRzI9neWagqNdwvYkQsEjgfbKbYK7p2CNTUQ
-----END CERTIFICATE-----
)EOF";
#endif // SECURE_MQTT

INA219 INA(0x40);

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

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

void setup_lora();
void displayInfo();
void sendGeo();
void setup_wifi();
void reconnect();
void setClock();
void callback(char* topic, byte* message, unsigned int length);
void onReceive(int packetSize);

// The TinyGPSPlus object
TinyGPSPlus gps;
// DC Motor object
Motor dcMotor;

//brake simulation, dirty hack
bool brakeState = false;
bool secondCarConnected = false;

#define PRESSURE_NOT_CONNECTED 0.0 //mbar
#define PRESSURE_APPLIED_BRAKES 4200.0 //mbar
#define PRESSURE_NOT_APPLIED_BRAKES 5000.0 //mbar

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, GPS_RX_UART_PIN, GPS_TX_UART_PIN);
  
  dcMotor.begin(MOTOR_PIN, PWM_MOTOR_PIN, pwmChannel, freq, resolution);

  if (!INA.begin() )
  {
    Serial.println("could not connect. Fix and Reboot");
  }

  INA.setMaxCurrentShunt(2.5, 0.002);

  setup_wifi();
  setup_lora();

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

// For stats that happen every 5 seconds
unsigned long last = 0UL;

void loop() {

  dcMotor.update();

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();

  if (now - lastMsg > 1000) 
  {

    while (Serial2.available() > 0)
    {
    if (gps.encode(Serial2.read()))
      {
        displayInfo();
        sendGeo();
      }
    }
  
    lastMsg = now;
    
    /*Ina sensor*/
    float voltage = INA.getBusVoltage();
    float current = abs(INA.getCurrent_mA());
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


    // brake pressure simulation, use when radio is not woring
    float pressure_one = 0.0;
    float pressure_two = 0.0;

    if (brakeState)
    {
      pressure_one = PRESSURE_APPLIED_BRAKES;
      if (secondCarConnected)
      {
        pressure_two = PRESSURE_APPLIED_BRAKES;
      }
      else
      {
        pressure_two = PRESSURE_NOT_CONNECTED;
      }
    }
    else
    {
      pressure_one = PRESSURE_APPLIED_BRAKES;
      if (secondCarConnected)
      {
        pressure_two = PRESSURE_NOT_APPLIED_BRAKES;
      }
      else
      {
        pressure_two = PRESSURE_NOT_CONNECTED;
      }
    }

    dtostrf(pressure_one, 1, 2, tempString);
    client.publish("loco/brake/pressure/1/", tempString);
    dtostrf(pressure_two, 1, 2, tempString);
    client.publish("loco/brake/pressure/2/", tempString);
  }

  if (millis() > 5000 && gps.charsProcessed() < 10)
  {
    Serial.println(F("No GPS detected: check wiring."));
    while(true);
  }
}

void setup_lora()
{
  Serial.println("LoRa Receiver");
  // 433E6
  // put the right pin numbers or -1 for default. On my TTGO it's 5, 19, 27, 18
  int counter = 0;
  LoRa.setPins(RADIO_SS, RADIO_RST, RADIO_DIO0);

  while (!LoRa.begin(433E6) && counter < 5) 
  {
    Serial.print(".");
    counter++;
    delay(100);
  }
  if (counter >= 5) 
  {
    // Increment readingID on every new reading
    Serial.println("Starting LoRa failed!"); 
  }
  else
  {
    Serial.println("Lora started!"); 
    LoRa.setSyncWord(0xF3);
    // register the receive callback
    LoRa.onReceive(onReceive);
    // put the radio into receive mode
    LoRa.receive();
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
      dcMotor.enable();
    }
    else if(messageTemp == "off"){
      Serial.println("off");
      dcMotor.disable();
    }
  }
  if (String(topic) == "loco/control/motor/power") {
    int motorSpeed = messageTemp.toInt();
    dcMotor.set_speed(motorSpeed);
  }
  if (String(topic) == "loco/control/brake/switch") {
    Serial.print("Changing brake state to ");
    if(messageTemp == "on"){
      Serial.println("on");
      brakeState = true;
    }
    else if(messageTemp == "off"){
      Serial.println("off");
      brakeState = false;
    }
  }

}

void setClock()
{
  configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");

  Serial.print("Waiting for NTP time sync: ");
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println("");
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  Serial.print("Current time: ");
  Serial.print(asctime(&timeinfo));
}

void onReceive(int packetSize) {
  // received a packet
  Serial.print("Received packet '");

  // read packet
  for (int i = 0; i < packetSize; i++) {
    Serial.print((char)LoRa.read());
  }

  // print RSSI of packet
  Serial.print("' with RSSI ");
  Serial.println(LoRa.packetRssi());
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
      client.subscribe("loco/control/brake/switch");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
