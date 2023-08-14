#include <SoftwareSerial.h>
#include "LocalDefines.h"
#define SSID SSID_LOCAL
#define PASS PASS_LOCAL

// When debugging these need to be RX -> RX and Tx -> Tx, (USB) but when writing as the arduino it's a switch!
SoftwareSerial espSerial(10, 11); // RX, TX

int const led = 9;
int const debugLed = 13;
char myChar;

void setup()
{
  pinMode(led, OUTPUT);
  pinMode(debugLed, OUTPUT);

  Serial.begin(9600);
  Serial.setTimeout(2000);

  //blink debugLed to indicate power up
  for (int i = 0; i < 15; i++)
  {
    digitalWrite(debugLed, HIGH);
    delay(50);
    digitalWrite(debugLed, LOW);
    delay(50);
  }

  // Open serial communications and wait for port to open:
  espSerial.begin(115200);
  espSerial.setTimeout(2000);
  espSerial.listen();

  if(espSerial.isListening()){
    Serial.println("Is listening!");
  }


  Serial.println("ESP8266 Demo");
  delay(100);

  //test if the module is ready
  
  int nmbrWritten = espSerial.println("AT+RST");
  Serial.print("Bytes written:");
  Serial.println(nmbrWritten);
  
  // while (espSerial.available() == 0); //wait for data
  espSerial.listen();
  
  if (espSerial.available() > 0) {
    // read the incoming byte:
    int incomingByte = espSerial.read();
    // say what you got:
    Serial.print("I received: ");
    Serial.println(incomingByte, DEC);
   }
   else{
    Serial.println("ESP not ready!");
   }

   Serial.println(espSerial.peek());

  espSerial.println("AT+RST");
  if (espSerial.find("ready"))
  {
    Serial.println("Module is ready");
    delay(1000);
    //connect to the wifi
    boolean connected = false;
    for (int i = 0; i < 5; i++)
    {
      if (connectWiFi())
      {
        connected = true;
        break;
      }
    }
    if (!connected)
    {
      //die
      while (1);
    }

    delay(5000);
    //set the single connection mode
    espSerial.println("AT+CIPMUX=0");
  }
  else
  {
    Serial.println("Module didn't respond.");
    delay(100);

    //serial loop mode for diag
    while (1) {
      while (Serial.available()) {
        myChar = Serial.read();
        espSerial.print(myChar);
        digitalWrite(13, HIGH);
        delay(50);
        digitalWrite(13, LOW);
        delay(50);
      }

      while (espSerial.available()) {
        myChar = espSerial.read();
        delay(25);
        Serial.print(myChar);
      }
    }
  }
}

void loop()
{
  String cmd = "AT+GMR";
  cmd += "\r\n\r\n";

  espSerial.print("AT+CIPSEND=");
  espSerial.println(cmd.length());
  if (espSerial.find(">"))
  {
    // dbgSerial.print(">");
  }
  else
  {
    espSerial.println("AT+CIPCLOSE");
    Serial.println("connect timeout");
    delay(1000);
    return;
  }

  int  charCount = 0;
  String statusStr = "";
  espSerial.print(cmd);

  if ( espSerial.find("status: ")) {
    char c;
    while (espSerial.available() == 0); //wait for data
    while (espSerial.available())
    {
      c = espSerial.read();
      if (charCount < 3)
        statusStr += c;
      else if (charCount > 99) //avoid reading noise forever just in case
        break;

      charCount++;
      delay(50); //wait for more data. fixme: can this be smaller?
    }

    if (charCount > 0)
    {
      //Adjust the LED brightness
      analogWrite(led, statusStr.toInt() );

      Serial.print("status=");
      Serial.println(statusStr);
    }
  }
  Serial.println();
  Serial.println("====");
  delay(1000);
}

boolean connectWiFi()
{
  espSerial.println("AT+CWMODE=1");
  String cmd = "AT+CWJAP=\"";
  cmd += SSID;
  cmd += "\",\"";
  cmd += PASS;
  cmd += "\"";
  Serial.println(cmd);
  espSerial.println(cmd);
  delay(2000);
  if (espSerial.find("OK"))
  {
    Serial.println("OK, Connected to WiFi.");
    return true;
  }
  else
  {
    Serial.println("Can not connect to the WiFi.");
    return false;
  }
}
