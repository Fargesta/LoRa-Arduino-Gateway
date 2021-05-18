#include <Arduino.h>
#include <SPI.h>
#include <RH_RF95.h>

#define RF95_FREQ 868.1
//#define RF95_FREQ 868.3
//#define RF95_FREQ 868.5
//#define RF95_FREQ 867.1
//#define RF95_FREQ 867.3
//#define RF95_FREQ 867.5
//#define RF95_FREQ 867.7
//#define RF95_FREQ 867.9
//#define RF95_FREQ 868.8
//#define RF95_FREQ 869.525

#define RFM95_RST 9

RH_RF95 rf95;
int16_t packetNum = 0;

void setup()
{
  pinMode(RFM95_RST, OUTPUT);

  digitalWrite(RFM95_RST, HIGH);

  Serial.begin(9600);
  while (!Serial);
  delay(100);

  Serial.println("Starting LoRa Sender");

  //manual reset
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);

  while(!rf95.init())
  {
    Serial.println("LoRa radio init failed");
    while (1);
  }
  Serial.println("LoRa radio init OK!");

  if(!rf95.setFrequency(RF95_FREQ))
  {
    Serial.println("setFrequency failed");
    while(1);
  }
  Serial.print("Set freq to: ");
  Serial.println(RF95_FREQ);

  rf95.setTxPower(23, false);
}

void loop()
{
  //Send message
  Serial.println("Sending to nodes");

  char radioPacket[20] = "Hello World #      ";
  itoa(packetNum++, radioPacket + 13, 10);
  Serial.print("Sending");
  Serial.println(radioPacket);
  radioPacket[19] = 0;

  Serial.println("Sending...");
  delay(10);
  rf95.send((uint8_t *)radioPacket, 20);

  Serial.println("Waiting for packet to complete...");
  delay(10);
  rf95.waitPacketSent();

  //Wait response
  uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
  uint8_t len = sizeof(buf);

  Serial.println("Waiting for reply");
  delay(10);
  if(rf95.waitAvailableTimeout(1000))
  {
    //get message
    if(rf95.recv(buf, &len))
    {
      Serial.print("Got reply: ");
      Serial.println((char*)buf);
      Serial.print("RSSI: ");
      Serial.println(rf95.lastRssi(), DEC);    
    }
    else
    {
      Serial.println("Receive failed");
    }
  }
  else
  {
    Serial.println("No reply, is there a listener around?");
  }
  delay(1000);
}