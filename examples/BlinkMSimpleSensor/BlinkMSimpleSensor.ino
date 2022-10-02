/*
 * BlinkMSimpleSensor -- simplest BlinkM example.
 *   Read a sensor no analog pin out and 
 *   change the hue of the BlinkM
 * 
 */

#include "BlinkM.h"
#include "Wire.h"


int sensorPin = 0;    // select the input pin for the potentiometer
int ledPin = 13;      // select the pin for the LED
int sensorValue = 0;  // variable to store the value coming from the sensor

int blinkm_addr = 0;  // 0 = talk to all blinkms on the i2c bus

BlinkM blinkm = BlinkM( blinkm_addr );

//
void setup() 
{
  // declare the ledPin as an OUTPUT:
  pinMode(ledPin, OUTPUT);
  pinMode(sensorPin, INPUT_PULLUP);

  blinkm.powerUp();  // use Analog pins A3 & A3 as power pins

  blinkm.begin();
  blinkm.stopScript();  // turn off startup script

  Serial.begin(9600);
  Serial.println("BlinkSimpleMSensor ready");
}

//
void loop() 
{
    // read the value from the sensor:
    sensorValue = analogRead(sensorPin); 
    Serial.println(sensorValue);
    blinkm.fadeToHSB( sensorValue, 255, 255);
    delay(100);                  
}
