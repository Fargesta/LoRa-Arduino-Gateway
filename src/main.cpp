#include <Arduino.h>
#include <SPI.h>
#include <RH_RF95.h>
#include <main.h>

// new RF_95
RH_RF95 rf95;

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
}

void loop()
{
  //Send message if serial available
  while(Serial.available())
  {
    String serialRead = Serial.readString();
    if(!serialRead.equals(START))
    {
      String zoneName = serialRead.substring(0, 4);
      String zoneId = serialRead.substring(4, 8);
      String cmd = zoneName;
      cmd += zoneId;
      cmd += ID;
      cmd += serialRead.substring(8);
      short messageLength = cmd.length() + 1; //must be +1 for eof symbol
      char radioMessage[messageLength + 1];
      cmd.toCharArray(radioMessage, messageLength);

      Serial.print("GW> Sending command: ");
      Serial.println(cmd);
      radioMessage[messageLength] = 0;
      delay(10);

      //Encrypt here
      //...
      rf95.send((uint8_t *)radioMessage, messageLength);

      delay(10);
      rf95.waitPacketSent();
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
    if(rf95.available())
    {
      if(rf95.recv(buf, &len))
      {
        String msg = (char*)buf;
        if(msg.substring(0, 4).equals(NAME))
        {
          msg = msg.substring(4); //Remove name from message
          //Decrypt here
          //...
          if(msg.substring(0, 8).equals(ID))
          {
            msg = msg.substring(8); //Remove receiver ID from packet

            Serial.print("ZN>");
            Serial.print(msg);
            Serial.print("|rs");
            Serial.println(rf95.lastRssi(), DEC);
          }
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