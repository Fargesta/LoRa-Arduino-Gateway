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
      String cmd = NAME;
      cmd += ID;
      cmd += serialRead;
      short messageLength = cmd.length();
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
          msg = msg.substring(4);

          //Decrypt here
          //...
          Serial.print("Zone> ");
          Serial.println(msg);
          Serial.print("rs");
          Serial.println(rf95.lastRssi(), DEC);
        }
      }
      else
      {
        Serial.println("GW> ERROR Read buffer");
      }
    }
    delay(10);
  }
}