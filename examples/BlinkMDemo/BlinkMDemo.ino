/*
 * BlinkMStart -- simplest BlinkM example.
 *   Change the hue of the BlinkM
 *
 */

#include "BlinkM.h"
#include "Wire.h"


const int ledPin = 13;      // select the pin for the LED
const int buttonPin = 6;    // pin that button is hooked to

int blinkm_addr = 0;  // 0 = talk to all blinkms on the i2c bus

byte hue = 0;

unsigned long previousMillis;

BlinkM blinkm = BlinkM( blinkm_addr );

//
void setup()
{
  delay(1000);
  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);

  blinkm.powerUp();  // use analog pins A3,A2 as power pins (works for Uno)

  blinkm.begin();
  blinkm.off();     // stop script and go to black

  delay(1000);
  Serial.begin(9600);
  Serial.println("BlinkDemo ready");
}

//
void loop()
{
  if( digitalRead(buttonPin) == LOW ) { // button pushed
    Serial.println("button pushed!");
    blinkm.setFadeSpeed(255);
    blinkm.fadeToRGB(255,255,255);
    delay(500);
    blinkm.setFadeSpeed(10);
    blinkm.fadeToRGB(0,0,0);
    delay(750);
  }
  
  if( (millis() - previousMillis) > 200 ) {
    previousMillis = millis();  
    hue += 10; // random(0,255);
    Serial.print("Fading to hue ");
    Serial.println(hue);

    blinkm.fadeToHSB( hue, 255, 255);
  }

}
