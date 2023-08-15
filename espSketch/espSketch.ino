#include <SoftwareSerial.h>
#include "LocalDefines.h"

#define SSID SSID_LOCAL
#define PASS PASS_LOCAL
#define IP IP_LOCAL
#define HOST HOST_LOCAL
#define PORT PORT_LOCAL

// When debugging these need to be RX -> RX and Tx -> Tx, (USB) but when writing as the arduino it's a switch!
SoftwareSerial espSerial(10, 11); // RX, TX

int const led = 9;
int const debugLed = 13;
char myChar;
String ip_own = "";
bool message_sent = false;
bool cip_server_started = false;

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
  espSerial.println("AT+RST");
  
  int charCount = 0;
  char c;
  String statusStr = "";

  while (espSerial.available() == 0); // wait for data
  while (espSerial.available())
  {
      c = espSerial.read();
      statusStr += c;
      if (espSerial.available()==0){
        break;
      }

      charCount++;
      delay(50); //wait for more data. fixme: can this be smaller?
   }
   Serial.println(statusStr);

  espSerial.println("AT+RST");

  if (espSerial.find("ready"))
  {
    Serial.println("Module is ready");
    delay(1000);
    //connect to the wifi
    boolean connected = false;
    for (int i = 0; i < 5; i++)
    {
      if (connect_wifi())
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
    espSerial.println("AT+CIPMUX=1");
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
  if( ip_own == ""){
    ip_own = get_own_ip();
    ip_own.replace("/", ".");
    Serial.println(ip_own);
    delay(100);
    if(ip_own == ""){
        Serial.println("Unable to get own ip");
        return;
    }
  }

  //if(!cip_start(IP, PORT)){
  if(!cip_server_started){
    cip_server_started = cip_start("1", 80);
    if(!cip_server_started){
        delay(100);
        Serial.println("Unable to cip start");
        return;
    }
  }

  if(!message_sent){
    message_sent = send_message("hello");
    if(!message_sent){
        delay(100);
        Serial.println("Unable to send message");
        return;
    }
  }

  return;



  //if(!get_status(HOST)){
  //  delay(100);
  //  Serial.println("Unable to get status");
  //  return;
  //}

  int  charCount = 0;
  String statusStr = "";
  //espSerial.print(cmd);

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


boolean cip_start(String dst_ip, int port)
{
    //String cmd = "AT+CIPSTART=\"TCP\",\"";
    String cmd = "AT+CIPSERVER=";
    cmd += dst_ip;
    cmd += "," + String(port);


    Serial.println(cmd);
    espSerial.println(cmd);

    delay(1000);
    while(espSerial.available() == 0){
        delay(100);
    }

    Serial.println(espSerial.readStringUntil("\n"));

    if(espSerial.find("Error")){
        Serial.println("\"" + cmd + "\"");
        Serial.println("Yielded Error!");
        return false;
    }
    Serial.println(espSerial.readStringUntil("\n"));

    return true;
}

String get_own_ip()
{
    espSerial.println("AT+CIFSR");
    while(espSerial.available() == 0);
    delay(1000);
    int count = 0;
    while(espSerial.available() > 0 ){
       String query = espSerial.readStringUntil("\n");
       Serial.println(query);
       if(query.startsWith("CIFSR:STAIP")){
            String out = query.substring(12);
            Serial.println("out: " + out);
            return out;
       }
       Serial.println(count++);
    }
    return "";
}

boolean send_message(String message)
{
    espSerial.print("AT+CIPSEND=0,");
    espSerial.println(message.length());
    espSerial.println(message);

    //delay(1000);
    while(espSerial.available() == 0){
        delay(100);
    }

    if(!espSerial.find("OK")){
      Serial.println("Unable to send " + message);
      espSerial.println("AT+CIPCLOSE=0");
      return false;
    }
    Serial.println("Sent!");
    espSerial.println("AT+CIPCLOSE=0");
    return true;

}

boolean get_status(String host)
{
  //String cmd = "GET /status HTTP/1.0\r\nHost: ";
  //cmd += host;
  //cmd += "\r\n\r\n";

  String cmd = "hello";
  espSerial.print("AT+CIPSEND=0,");
  espSerial.println(cmd.length());
  if(!espSerial.find(">")){
    Serial.println(espSerial.readStringUntil("\r\n"));
    Serial.println("Connection Timeout!");
    espSerial.println("AT+CIPCLOSE");
    delay(1000);
    return false;
  }
  espSerial.println(cmd);

  if(!espSerial.find("OK")){
    Serial.println("Unable to send " + cmd);
    return false;
  }
  Serial.println("Sent!");

  espSerial.println("AT+CIPCLOSE=0");
  return true;
}

boolean connect_wifi()
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
