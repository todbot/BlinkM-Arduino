/*
 * BlinkMMulti -- 
 *
 *
 * BlinkM connections to Arduino
 * PWR - -- gnd -- black -- Gnd
 * PWR + -- +5V -- red   -- 5V
 * I2C d -- SDA -- green -- Analog In 4
 * I2C c -- SCK -- blue  -- Analog In 5
 *
 *
 * 2007, Tod E. Kurt, ThingM, http://thingm.com/
 *
 */


#include "BlinkM.h"
#include "Wire.h"

byte cmd;

char serInStr[30];  // array that will hold the serial input string

BlinkM blinkm = BlinkM();

void help()
{
    Serial.println("\r\nBlinkMMulti!\n"
                   "'A <n>'     -- change address to <n>\n"
                   "'h <n> <h>' -- set hue on <n> to <h>\n"
                   "'o <n>'     -- stop script on <n>\n"
                   "'O <n>'     -- turn off blinkm <n>\n"
                   "'p <n> <p>' -- play script <p> on <n>\n"
                   "'f <n> <f>' -- set fadespeed on <n> to <f>\n"
                   "'t <n> <t>' -- set timeadj on <n> to <t>\n"
                   "Note: address 0 is broadcast address\n"
                   );
}

void setup()
{
    blinkm.begin();

    Serial.begin(9600);

    help();
    Serial.print("cmd>");
}


void loop()
{
    if( !readSerialString() ) {  // did we read a string?
        return;
    }

    // yes we did. we can has serialz
    Serial.println(serInStr); // echo back string read
    char cmd = serInStr[0];  // first char is command
    char* str = serInStr+1;  // get me a pointer to the first char

    // most commands are of the format "addr num"
    byte addr = (byte) strtol( str, &str, 10 );
    byte num  = (byte) strtol( str, &str, 10 );  // this might contain 0

    Serial.print("addr ");
    Serial.print(addr,DEC);

    switch(cmd) {
    case '?': 
        help();
        break;
    case 'A':  // set Address
        if( addr>0 && addr<0xff ) {
            Serial.println(" setting address");
            blinkm.changeAddress(addr);
        } else { 
            Serial.println("bad address");
        }
        break;
    case 'h':  // set hue
        Serial.print(" to hue "); 
        Serial.println(num,DEC);
        blinkm.fadeToHSB( num, 0xff, 0xff, addr );
        break;
    case 'f':  // set fade speed
        Serial.print(" to fadespeed "); 
        Serial.println(num,DEC);
        blinkm.setFadeSpeed( num, addr );
        break;
    case 't':  // set time adj
        Serial.print(" to time adj "); 
        Serial.println(num,DEC);
        blinkm.setTimeAdj( num, addr );
        break;
    case 'o':   // stop script
        Serial.println(" stopping script");
        blinkm.stopScript(addr);
        break;
    case 'O':  // off
      Serial.println(" turning off blinkm");
      blinkm.fadeToRGB(0,0,0, addr);
      break;
    case 'p':   // play script
        Serial.print(" playing script ");
        Serial.println(num,DEC);
        blinkm.playScript(num, 0x00,0x00, addr);
        break;
    default: 
        Serial.println(" unknown cmd");
    } // case

    Serial.print("cmd> ");
}

//read a string from the serial and store it in an array
//you must supply the array variable
uint8_t readSerialString()
{
    if(!Serial.available()) {
        return 0;
    }
    delay(10);  // wait a little for serial data
    int i = 0;
    while (Serial.available()) {
        serInStr[i] = Serial.read();   // FIXME: doesn't check buffer overrun
        i++;
    }
    serInStr[i] = 0;  // indicate end of read string
    return i;  // return number of chars read
}

