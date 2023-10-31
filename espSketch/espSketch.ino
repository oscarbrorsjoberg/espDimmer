#include <SoftwareSerial.h>
#include <Arduino.h>

// small header containing local info
#include "LocalDefines.h"

#define SSID SSID_LOCAL
#define PASS PASS_LOCAL
#define IP IP_LOCAL
#define HOST HOST_LOCAL
#define PORT PORT_LOCAL

// When debugging these need to be RX -> RX and Tx -> Tx, (USB) but when writing as the arduino it's a switch!
// So it's better to have the dbgSerial rewired
// I initially used this as the main line to communicate between esp and ard but there was too much noise!

SoftwareSerial dbgSerial(10, 11); // RX, TX

int const led = 9;
int const debugLed = 13;
String ip_own = "";
bool message_sent = false;
bool cip_server_started = false;

unsigned long start_time = millis();
unsigned long timeout = 4000;

boolean DEBUG = true; 

/*
    sends message and reads result
    "" if no message, 
    readString cleans RX buffer so can't do anything with it after
*/
String read_message(String message){

    if(DEBUG) dbgSerial.println("Sending message " + message);

    Serial.println(message);
    unsigned long message_start = millis();
    while(!Serial.available()){
        if((millis() - message_start) > timeout){
            if(DEBUG) dbgSerial.println("Timeout waiting for response!");
            break;
        }
    }

    String out = "";
    int av = 0;

    while(Serial.available()){
        if(DEBUG) dbgSerial.println("RX buffer av (" + String(av++) + ")");
        out += Serial.readStringUntil("\n");
    }
    return out;
}

void setup()
{
  pinMode(led, OUTPUT);
  pinMode(debugLed, OUTPUT);

  dbgSerial.begin(9600);
  dbgSerial.setTimeout(1000);


  //blink debugLed to indicate power up
  for (int i = 0; i < 15; i++)
  {
    digitalWrite(debugLed, HIGH);
    delay(50);
    digitalWrite(debugLed, LOW);
    delay(50);
   }

  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  Serial.setTimeout(4000);

  dbgSerial.println(" -- ESP8266 Demo -- ");
  delay(100);

  //test if the module is ready
  Serial.println("AT+RST");

  if (Serial.find("ready"))
  {
    dbgSerial.println("Module is ready");
    delay(1000);

    //connect to the wifi
    boolean connected = false;
    int retries = 10;

    while(!connected & (retries-- > 0))
        connected = connect_wifi();
    if (!connected)
    {
      //die
      while (1){
        digitalWrite(debugLed, HIGH);
        delay(100);
        digitalWrite(debugLed, LOW);
        delay(100);

      }
    }

    // set the single connection mode
    dbgSerial.println("Connected!");
    String cpmux_mes = read_message("AT+CIPMUX=1");

    if(cpmux_mes == ""){
        dbgSerial.println("Unable to get status after CIPMUX");
    }
    else{
        dbgSerial.println("=== message start ===");
        dbgSerial.println(cpmux_mes.trim());
        dbgSerial.println("=== message end ===");
    }
  }
  else
  {
      dbgSerial.println("Non-ready after reset");
      // die
      while(1){
          digitalWrite(debugLed, HIGH);
          delay(100);
          digitalWrite(debugLed, LOW);
          delay(100);
    }
  }
}

void loop()
{

  if( ip_own == ""){
    ip_own = get_own_ip();
    dbgSerial.println(ip_own);
    if(ip_own == ""){
        dbgSerial.println("Unable to get own ip");
        return;
    }
  }

  if(!cip_server_started){
    cip_server_started = cip_start(1, 80);
    if(!cip_server_started){
        delay(100);
        dbgSerial.println("Unable to cip start");
        return;
    }
    dbgSerial.println("Server started at " + ip_own);
  }

  if(!message_sent){
    message_sent = send_message("hello");
    if(!message_sent){
        delay(100);
        dbgSerial.println("Unable to send message");
        return;
    }
  }

  return;

  int  charCount = 0;
  String statusStr = "";

  if ( Serial.find("status: ")) {
    char c;
    while (Serial.available() == 0); //wait for data
    while (Serial.available())
    {
      c = Serial.read();
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

      dbgSerial.print("status=");
      dbgSerial.println(statusStr);
    }
  }
  Serial.println();
  dbgSerial.println("====");
  delay(1000);
}

/*
Open server mode (0 - closed, 1 - open) at port 
*/
boolean cip_start(int server_mode, int port)
{
    //String cmd = "AT+CIPSTART=\"TCP\",\"";
    String cmd = "AT+CIPSERVER=";
    cmd += String(dst_ip);
    cmd += "," + String(port);


    dbgSerial.println(cmd);
    Serial.println(cmd);
    while(Serial.available() == 0);

    // this cleans the rx buffer?
    if(Serial.find("Error")){
        dbgSerial.println("\"" + cmd + "\"");
        dbgSerial.println("Yielded Error!");
        return false;
    }

    return true;
}

String get_own_ip()
{
    Serial.println("AT+CIFSR");
    while(Serial.available() == 0);
    delay(1000);
    int count = 0;
    while(Serial.available() > 0 ){
       String query = Serial.readStringUntil("\n");
       dbgSerial.println(query);
       if(query.startsWith("CIFSR:STAIP")){
            String out = query.substring(12);
            dbgSerial.println("out: " + out);
            return out;
       }
       dbgSerial.println(count++);
    }
    return "";
}

boolean send_message(String message)
{
    dbgSerial.println("AT+CIPSEND=0," + String(message.length()));
    Serial.println("AT+CIPSEND=0," + String(message.length()));

    if (Serial.find(">"))
    {
        dbgSerial.print(">");
    }
    else
    {
        Serial.println("AT+CIPCLOSE");
        dbgSerial.println("connect timeout");
        delay(1000);
        return false;
    }

    Serial.println(message);

    unsigned long message_start = millis();
    while(!Serial.available()){
        if((millis() - message_start) > timeout){
            dbgSerial.println("Timeout waiting for response!");
            break;
        }
    }

    if(!Serial.available()){
       dbgSerial.println("Serial not avab");
       Serial.println("AT+CIPCLOSE=0");
       return false;
    }

    if(!Serial.find("OK")){
      dbgSerial.println("Did not find ok");
      dbgSerial.println(Serial.readString());
      Serial.println("AT+CIPCLOSE=0");
      return false;
    }

    dbgSerial.println("Sent!");
    Serial.println("AT+CIPCLOSE=0");
    return true;

}

boolean get_status(String host)
{
  //String cmd = "GET /status HTTP/1.0\r\nHost: ";
  //cmd += host;
  //cmd += "\r\n\r\n";

  String cmd = "hello";
  Serial.print("AT+CIPSEND=0,");
  Serial.println(cmd.length());
  if(!Serial.find(">")){
    dbgSerial.println(Serial.readStringUntil("\r\n"));
    dbgSerial.println("Connection Timeout!");
    Serial.println("AT+CIPCLOSE=0");
    delay(1000);
    return false;
  }

  Serial.println(cmd);
  if(!Serial.find("OK")){
    dbgSerial.println("Unable to send " + cmd);
    Serial.println("AT+CIPCLOSE=0");
    return false;
  }
  dbgSerial.println("Sent!");

  Serial.println("AT+CIPCLOSE=0");
  return true;
}

boolean connect_wifi()
{
  String cwmode_stat = read_message("AT+CWMODE=1");

  if(cwmode_stat == ""){
    dbgSerial.println("Unable to set CWMODE!");
    return false;
  }
  else{
      dbgSerial.println("=== message start ===");
      dbgSerial.println(cwmode_stat.trim());
      dbgSerial.println("=== message end ===");
  }

  String cmd = "AT+CWJAP=\"";
  cmd += SSID;
  cmd += "\",\"";
  cmd += PASS;
  cmd += "\"";
  dbgSerial.println(cmd);
  Serial.println(cmd);
  delay(2000);
  if (Serial.find("OK"))
  {
    dbgSerial.println("OK, Connected to WiFi.");
    return true;
  }
  else
  {
      dbgSerial.println("Can not connect to the WiFi.");
      return false;
  }
}
