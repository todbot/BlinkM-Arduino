/*
 * BlinkMStart -- simplest BlinkM example.
 *   Change the hue of the BlinkM
 * 
 */

#include "BlinkM.h"
#include "Wire.h"


int ledPin = 13;      // select the pin for the LED

int blinkm_addr = 0;  // 0 = talk to all blinkms on the i2c bus

byte hue=0;

BlinkM blinkm = BlinkM( blinkm_addr );

//
void setup() 
{
  delay(1000);
  pinMode(ledPin, OUTPUT);
 
  blinkm.powerUp();  // use analog pins A3,A2 as power pins (works for Uno)

  blinkm.begin();
  blinkm.off();     // stop script and go to black
  
  delay(1000);
  Serial.begin(9600);
  Serial.println("BlinkStart ready");
}

//
void loop() 
{
    hue += 10; // random(0,255);
    Serial.print("Fading to hue ");
    Serial.println(hue);
    
    blinkm.fadeToHSB( hue, 255, 255);
    
    delay(500);
}
