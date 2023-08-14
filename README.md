# Potem or the iot-lamp (web app and all)

# What is tty

What would one use tty instead of a terminal emulator?
Tellytype? Tellytype terminals?
There are seven ttys on your system, this I know.

Communicating over tty? ttyUSB like?


# The electronics

## The transistor

We're using a [IRF520](./irf520.pdf) transistor.

# ESP8266-1

## General rant
So I've had quite some problems following the general youtube tutorials.
The one that finally worked for me was this one. 
This tutorial didn't mind that the TX->RX line was 5v. And the ESP board does not seem to complain.

Downloading different boards for the arduino-cli (esp8266) didn't work either..
And communicating with the TTL unit didn't work either.  
It might just have been me fucking up, I'll give it another try right away.

## GTK AT Commands

Where we get stuff:
https://www.instructables.com/Getting-Started-With-the-ESP8266-ESP-01/
Similar
https://www.instructables.com/Get-Started-With-ESP8266-Using-AT-Commands-Via-Ard/


The ESP8266-01 module has three operational modes:

1. Access Point (AP)
2 way communication over wifi.

2. Station (STA)
ESP01 can connect to an AP. Other devices on this AP can communicate with module.

3. Both



# The application

## The web app

## What is the sketch.json
