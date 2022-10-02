/*
 * BlinkMAnalogSensor 
 *  -- read a sensor no analog pin out and  change the BlinkM's hue
 * 
 */

#include "BlinkM.h"
#include "Wire.h"


int sensorPin = 0;    // select the input pin for the potentiometer
int ledPin = 13;      // select the pin for the LED

int sensorValue = 0;  // variable to store the value coming from the sensor

int blinkm_addr = 0;  // 0 = broadcast, talk to all blinkms

BlinkM blinkm = BlinkM( blinkm_addr );


void setup() 
{
  // declare the ledPin as an OUTPUT:
  pinMode(ledPin, OUTPUT);
  pinMode(sensorPin, INPUT_PULLUP); // pullup so we can use button if we want

  blinkm.powerUp();
  blinkm.begin(); 
  blinkm.stopScript( );  // turn off startup script

  Serial.begin(9600);
  Serial.println("BlinkMAnalogSensor ready");
}

void loop() 
{
    // read the value from the sensor
    sensorValue = analogRead(sensorPin);

    // convert from 0-1023 to 0-255
    sensorValue = sensorValue / 4 ;

    // use it as hue value to BlinkM
    blinkm.fadeToHSB( sensorValue, 255, 255 );

    // print out the value if you want to look at it
    Serial.println(sensorValue);
    delay(100);                  
}
