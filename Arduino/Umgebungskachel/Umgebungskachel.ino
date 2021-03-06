/**
    *** Umgebungskachel.ino ***
    Implements a sensor demonstrator for the Qivicon platform on a Intel Galileo Board (Arduino).
    This file contains the imports, global variables and also the loop and setup methods that are essential for an Arduino sketch.
**/
// Library Include of the generated Code
#include "SensorState.h"


#include "ArduinoJson.h"
#include <Wire.h>
#include "rgb_lcd.h"
#include "DHT.h"
#include <Ethernet.h>
#include <math.h>
#include <TimerOne.h>



//Define Sensor And Output Pins
#define LED 8
#define LIGHT_SENSOR 0
#define TEMP_SENSOR 1
#define TOUCH_SENSOR 2
#define BUZZER 6

//Definitions for the DHT11 Sensor
#define DHTIN 4
#define DHTOUT 5
#define DHTTYPE DHT11

//Start dht library
DHT dht(DHTIN,DHTOUT, DHTTYPE);

//LCD Display
rgb_lcd lcd;

//Network connection
boolean ethernetConnection = false;

SensorState sensorState;


// the media access control (ethernet hardware) address for the shield:
byte MAC[] = { 0x98, 0x4F, 0xEE, 0x05, 0x4C, 0x74 };

// server  listens to port 80
EthernetServer server(80);

/*
* Setup method is called before the programm indefinitly repeats the loop() function
*/
void setup()
{
 //Setup the Display and Display the current action
 lcd.begin(16,2); 
 lcd.setRGB(255,0,0); 
 
 //First try to connect via Wifi
 showDisplay("Initializing ...","Wireless!"); 
 
 //Two separated Calls to the linux system, to activate the wlan interface
 system("ifup wlan0");
 delay(3000);
 system("ifup wlan0");
 delay(3000);
 
 if(getWifiIP() == "no IP") {
   //No wifi connection. Try connecting via ethernet
   ethernetConnection = true;
   showDisplay("Initializing ...","Ethernet!"); 
   //Call to the linux system, to activate the ethernet port
   system("ifup eth0");
   delay(5000);  
 }
 
 // set LED pin on OUTPUT and turn it off
 pinMode(LED, OUTPUT);
 digitalWrite(LED, LOW);
 //set Buzzer pin to OUTPUT
 pinMode(BUZZER, OUTPUT);

 // open the serial port at 9600 baud 
 updateLowerDisplay("Serial port!");
 Serial.begin(9600);
 Serial.println("Serial setup!");
 
 // wait for connection to network and IP assignment
 updateLowerDisplay("DHCP Protocol!");
 Ethernet.begin(MAC);
 server.begin();
 
 showDisplay("Server is at:",(ethernetConnection)? getLocalIP() : getWifiIP());
 lcd.setRGB(150,255,0);
 
 //Setup Dht Library
 dht.begin(); 
 
 //initialize Datastructure and values of the thresholds
 initDatastructure();
 
 //setup an interrupt for a touch event
 attachInterrupt(TOUCH_SENSOR, touchEvent, CHANGE);
 
 //Initialize the TimerOne Library with a 1s Period
 Timer1.initialize(1000000); 
 Timer1.attachInterrupt(isr,1000000); //attach a TimerInterrupt to call the isr method every 1 seconds
 
}

/**
  The loop contains the main code of the Sketch which is handling the REST Server.
**/
void loop()
{
  
 // listen for incoming clients
 EthernetClient client = server.available();
 
 if (client) {
  
  //get the request Data
  String content = readClientRequest(client);
  
  // Call the parseFormJson function of the sensidl library
  if(!content.equals(""))
    parseDatastructureFromJson(content);
  
  //refresh LED acccording to recieved data
  if(get_SensorState_led(&sensorState) == "ON") {
    digitalWrite(LED, HIGH);  
  } else if (get_SensorState_led(&sensorState) == "OFF") {
    digitalWrite(LED, LOW);  
  }
  
  //send a http header
  sendHeader(client);
  
  // check current led state and update it
  if (digitalRead(LED) == HIGH) {
   set_SensorState_led(&sensorState,"ON");
  }
  else {
   set_SensorState_led(&sensorState,"OFF");
  }
  
  //Update the Temperature and the Light Resistance and refresh the lcd display
  updateSensorData();
  refreshDisplay();
  
  //write the generated json data to the client
  client.println(datastructureToJson());

  // give the web browser time to receive the data
  delay(1);
  // close the connection
  client.stop();
 }
}

/**
  Converts a double value to a String variable
  [ This method is a small adaptation of the dtostrf method that is implemented in c but not accessible with the Arduino IDE]
**/
String dtostrf (double val, signed char width, unsigned char prec) {
	char sout[24];
        char fmt[20];
	sprintf(fmt, "%%%d.%df", width, prec);
	sprintf(sout, fmt, val);
	return String(sout);
}

