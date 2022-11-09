#include <SPI.h>
#include <LoRa.h>

//define the pins used by the transceiver module
#define ss 5
#define rst 14
#define dio0 26

int counter = 0;

void setup() {
  Serial.begin(9600);
  while (!Serial);
  delay(500);
  Serial.println("LoRa Receiver");
  //433E6
  // put the right pin numbers or -1 for default. On my TTGO it's 5, 19, 27, 18
  int counter = 0;
  LoRa.setPins(ss, rst, dio0);
  while (!LoRa.begin(433E6) && counter < 10) 
  {
    Serial.print(".");
    counter++;
    delay(500);
  }
  if (counter == 10) 
  {
    // Increment readingID on every new reading
    Serial.println("Starting LoRa failed!"); 
  }
}

void loop() {
  // try to parse packet
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // received a packet
    Serial.print("Received packet '");

    // read packet
    while (LoRa.available()) {
      Serial.print((char)LoRa.read());
    }

    // print RSSI of packet
    Serial.print("' with RSSI ");
    Serial.println(LoRa.packetRssi());
  }
}