/*
 * RGB 3 Pot Mixer
 * ---------------
 *
 * Code for making three potentiometers control 3 LEDs, red, grn and blu, 
 * or one tri-color LED.
 * The program cross-fades from red to grn, grn to blu, and blu to red.
 * 
 * Code assumes you have the LEDs connected in a common-cathode configuration,
 * with the LED's cathode connected to GND via a resistor and the anode
 * connected to Arduino pins 9,10,11. 
 *
 * Tod E. Kurt, http://todbot.com/blog/
 *
 */

#include "BlinkM.h"
#include <Wire.h>


int DEBUG = 1;          // Set to 1 to turn on debugging output

// INPUT: Potentiometers should be connected to 5V and GND
const int potRPin = 0;
const int potGPin = 1;  
const int potBPin = 2;

// OUTPUT: Use digital pins 9-11, the Pulse-width Modulation (PWM) pins
const int redPin = 9;   // Red LED,   connected to digital pin 9
const int grnPin = 10;  // Green LED, connected to digital pin 10
const int bluPin = 11;  // Blue LED,  connected to digital pin 11

const int butPin = 12;

const int sampleSize = 8;

const int blinkm_addr = 0;  // all blinkms

// Program variables
int redVal;   // Variables to store the values to send to the pins
int grnVal;
int bluVal;

int redTot;
int grnTot;
int bluTot;


BlinkM blinkm = BlinkM( blinkm_addr );

void setup()
{
  pinMode(redPin, OUTPUT);   // sets the pins as output
  pinMode(grnPin, OUTPUT);   
  pinMode(bluPin, OUTPUT); 

  pinMode(butPin, INPUT_PULLUP);  // use internal pullup so press == LOW

  if (DEBUG) {          // If we want to see the pin values for debugging...
    Serial.begin(9600); 
    Serial.println("BlinkMRGB3PotMixer");
  }

  blinkm.begin();
  blinkm.stopScript();
  blinkm.off();

}

// Main program
void loop()
{
  redVal = analogRead(potRPin); 
  redVal = redVal/4;
  redTot += redVal - (redTot/sampleSize);
  redVal = redTot/sampleSize;  // take 16 samples
  redVal = (redVal < 5 ) ? 0 : redVal;

  grnVal = analogRead(potGPin); 
  grnVal = grnVal/4;
  grnTot += grnVal - (grnTot/sampleSize);
  grnVal = grnTot/sampleSize;  // take 16 samples
  grnVal = (grnVal < 5 ) ? 0 : grnVal;

  bluVal = analogRead(potBPin); 
  bluVal = bluVal/4;
  bluTot += bluVal - (bluTot/sampleSize);  // add new val, subtract 'old' val
  bluVal = bluTot/sampleSize; 
  bluVal = (bluVal < 5 ) ? 0 : bluVal;

 
  analogWrite(redPin, redVal);   // Write values to LED pins
  analogWrite(grnPin, grnVal); 
  analogWrite(bluPin, bluVal);  


  // button pressed
  if( digitalRead(butPin) == LOW ) { 
    redTot = 64*255;
    grnTot = 64*255;
    bluTot = 64*255;
    blinkm.setFadeSpeed( 30 );
    delay(100);
    blinkm.fadeToRGB( 0,0,0 );
    delay(100);
  }
  else {
      blinkm.fadeToRGB( redVal, grnVal, bluVal );
  }

  if (DEBUG) { // If we want to read the output
    DEBUG += 1;      // Increment the DEBUG counter
    if (DEBUG > 100) { // Print every hundred loops
      DEBUG = 1;     // Reset the counter
      Serial.print("R:");    // Indicate that output is red value
      Serial.print(redVal);  // Print red value
      Serial.print("\t");    // Print a tab
      Serial.print("G:");    // Repeat for grn and blu...
      Serial.print(grnVal);
      Serial.print("\t");    
      Serial.print("B:");    
      Serial.println(bluVal); // println, to end with a carriage return
    }
  }

  delay(20);

}
