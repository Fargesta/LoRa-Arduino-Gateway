#include <Arduino.h>
#include <SPI.h>
#include <RH_RF95.h>
#include <RHEncryptedDriver.h>
#include <Speck.h>
#include <main.h>

//Init driver and encrytion
RH_RF95 rf95;
Speck msgCipher;
RHEncryptedDriver encDriver(rf95, msgCipher);

//Start flag
bool isStarted = false;

void setup()
{
  pinMode(RFM95_RST, OUTPUT);

  digitalWrite(RFM95_RST, HIGH);

  Serial.begin(9600);
  while (!Serial);
  delay(100);

  Serial.println("GW> Starting LoRa Sender");

  //manual reset
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);

  while(!rf95.init())
  {
    Serial.println("GW> LoRa radio init failed");
    while (1);
  }
  Serial.println("GW> LoRa radio init OK!");

  if(!rf95.setFrequency(RF95_FREQ))
  {
    Serial.println("GW> setFrequency failed");
    while(1);
  }
  Serial.print("GW> Set freq to: ");
  Serial.println(RF95_FREQ);

  rf95.setTxPower(23, false);
  rf95.setSignalBandwidth(62500);
  rf95.setSpreadingFactor(10);
  encDriver.setCADTimeout(500);
  msgCipher.setKey(encryptKey, sizeof(encryptKey));
  encDriver.setThisAddress(NETWORK_ID);
}

void loop()
{
  //Send message if serial available
  while(Serial.available())
  {
    String serialRead = Serial.readString();
    if(!serialRead.equals(START))
    {
      uint8_t zoneNetworkId = serialRead.substring(0, 2).toInt();
      encDriver.setHeaderTo(zoneNetworkId);
      serialRead = serialRead.substring(2);
      String zonePassword = serialRead.substring(0, 8);
      String cmd = zonePassword;
      cmd += serialRead.substring(8);
      short messageLength = cmd.length() + 1; //must be +1 for eof symbol
      char radioMessage[messageLength + 1];
      cmd.toCharArray(radioMessage, messageLength);

      Serial.print("GW> Sending command: ");
      Serial.println(cmd);
      radioMessage[messageLength] = 0;
      delay(10);

      encDriver.send((uint8_t *)radioMessage, messageLength);
      delay(10);
      encDriver.waitPacketSent();
    }
    else
    {
      isStarted = true;
      Serial.println("GW> Gateway started");
    }
  }

  if(isStarted)
  {
    //Listen for response
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);

    //Listen messages
    if(encDriver.available())
    {
      if(encDriver.recv(buf, &len))
      {
        String msg = (char*)buf;
        if(msg.substring(0, 8).equals(PASSWORD))
        {
          msg = msg.substring(8); //Remove receiver ID from packet

          Serial.print("ZN>");
          Serial.print(msg);
          Serial.print("|rs");
          Serial.println(encDriver.lastRssi(), DEC);
        }
        msg = "";
      }
      else
      {
        Serial.println("GW> ERROR Read buffer");
      }
    }
    delay(10);
  }
}