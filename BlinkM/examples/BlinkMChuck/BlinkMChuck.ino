/*
 * BlinkMChuck -- Control a BlinkM with a Wii Nunchuck
 *
 *
 * BlinkM connections to Arduino
 * PWR - -- gnd -- black -- Gnd
 * PWR + -- +5V -- red   -- 5V
 * I2C d -- SDA -- green -- Analog In 4
 * I2C c -- SCK -- blue  -- Analog In 5
 *
 * Note: This sketch resets the I2C address of the BlinkM.
 *       If you don't want this behavior, comment out "BlinkM_setAddress()"
 *       in setup() and change the variable "blink_addr" to your BlinkM's addr.
 *
 * 2008 Tod E. Kurt, http://thingm.com/
 *
 */

#include "BlinkM.h"
#include "Wire.h"
#include "nunchuck_funcs.h"

int loop_cnt=0;

byte hue, bri;
byte zbut,cbut;
byte fadetoggle;
byte fast_fade = 30, slow_fade = 5;
int ledPin = 13;

BlinkM blinkm = BlinkM();

void setup()
{
    Serial.begin(9600);
    
    blinkm.powerUp();
    blinkm.begin();
    
    //BlinkM_setAddress( blinkm_addr );  // uncomment  to set address
    
    byte rc = blinkm.checkAddress();
    if( rc == -1 ) 
        Serial.println("\r\nno response");
    else if( rc == 1 ) 
        Serial.println("\r\naddr mismatch");

    blinkm.stopScript();
    blinkm.setFadeSpeed(slow_fade); 

    nunchuck_init(); // send the initilization handshake
    
    if( rc == 0 )
        Serial.print("BlinkMChuck ready\n");
    else 
        Serial.print("BlinkMChuck error initializing\n");
}


void loop()
{
  if( loop_cnt > 10 ) { // every 100 msecs get new data
        loop_cnt = 0;

        if( ! nunchuck_get_data() ) { 
            Serial.println("error getting nunchuck data");
        }

        hue  = nunchuck_accelx(); // ranges from approx 70 - 182
        bri  = nunchuck_accely(); // ranges from approx 65 - 173
        zbut = nunchuck_zbutton();
        cbut = nunchuck_cbutton(); 

        // C buton toggles fast or slow fading
        if( cbut ) {
            blinkm.setFadeSpeed( (fadetoggle)?fast_fade:slow_fade );
            fadetoggle = !fadetoggle;
            digitalWrite(ledPin, fadetoggle);
        }

        // Z button fades to black
        if( zbut ) {
            blinkm.fadeToHSB( hue, 0xff, 0); // fade to black
        } else {
            // massage a bit to get more range
            Serial.print("hue raw: "); Serial.print( hue,DEC);

            hue = map( hue, 60,190, 0,255);
            bri = map( bri, 60,180, 0,255);
            //hue = (hue-70) * 2.2;  // empirically determined
            //bri = (bri-60) * 2.2;  // empirically determined
            
            Serial.print("  hue: "); Serial.print( hue,DEC);
            Serial.print("\tri: "); Serial.println( bri,DEC);
            
            // set blinkm color
            blinkm.fadeToHSB( hue, 0xff, bri );
        }

    }
    loop_cnt++;
    delay(10);
}

