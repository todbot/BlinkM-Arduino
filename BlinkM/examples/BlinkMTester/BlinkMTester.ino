/*
 * BlinkMTester -- Simple "command-line" tool to play with BlinkMs
 *
 *  Once you load this sketch on to your Arduino, open the Serial Monitor
 *  and you'll see a menu of things to do.
 *
 *
 * BlinkM connections to Arduino
 * PWR - -- gnd -- black -- Gnd
 * PWR + -- +5V -- red   -- 5V
 * I2C d -- SDA -- green -- Analog In 4
 * I2C c -- SCK -- blue  -- Analog In 5
 *
 * Note: This sketch DOES NOT reset the I2C address of the BlinkM.
 *       If you want to change the I2C address use the 'A<n>' command.
 *
 * 2007-14, Tod E. Kurt, ThingM, http://thingm.com/
 *
 */


#include "BlinkM.h"
#include "Wire.h"

#include <avr/pgmspace.h>  // for progmem stuff
#include <stdlib.h>

// set this if you're plugging a BlinkM directly into an Arduino,
// into the standard position on analog in pins 2,3,4,5
// otherwise you can set it to false or just leave it alone
const boolean BLINKM_ARDUINO_POWERED = true;

const byte blinkm_addr_default = 0x09; // the default address of all BlinkMs

BlinkM blinkm = BlinkM();

const int serStrLen = 30;
char serInStr[ serStrLen ];  // array that will hold the serial input string

const char helpstr[] PROGMEM = 
  "'c<rrggbb>'  fade to an rgb color\n"
  "'h<hhssbb>'  fade to an hsb color\n"
  "'C<rrggbb>'  fade to a random rgb color\n"
  "'H<hhssbb>'  fade to a random hsb color\n"
  "'p<n>'  play a script\n"
  "'o'  stop a script\n"
  "'f<nn>'  change fade speed\n"
  "'t<nn>'  set time adj\n"
  "'g'  get current color\n"
  "'a'  get I2C address\n"
  "'A<n>'  set I2C address\n"
  "'B'  set startup params to default\n"
  "'Z'  get BlinkM version\n"
  "'i'  get input values\n"
  "'s'/'S'  scan i2c bus for 1st BlinkM / search for devices\n"
  "'R'  return BlinkM to factory settings\n"
  "'?'  for this help msg\n\n"
  ;
//const char badAddrStr[] PROGMEM = 
//  "BlinkM not at expected address.  Reset address with 'A' command\n";


// called when address is found in BlinkM_scanI2CBus()
void scanfunc( byte addr, byte result )
{
  Serial.print(F("addr: "));
  Serial.print(addr,DEC);
  Serial.print( (result==0) ? " found!":"       ");
  Serial.print( (addr%4) ? "\t":"\n");
}

// look for the first blinkm on the i2c bus and set up talk to it
void lookForBlinkM() 
{
  Serial.print(F("Looking for a BlinkM: "));
  int ad = blinkm.FindFirstI2CDevice();
  if( ad == -1 ) {
    Serial.println(F("No I2C devices found"));
  } else { 
    Serial.print(F("Device found at addr "));
    Serial.println( ad, DEC);
    //blinkm_addr = ad;
    blinkm.talkTo( ad );
  }
}

// arduino setup func
void setup()
{
  while(!Serial);
  Serial.begin(9600);
  Serial.println(F("\nBlinkMTester!"));

  if( BLINKM_ARDUINO_POWERED ) {
    blinkm.powerUp();
  }

  blinkm.begin();

  lookForBlinkM();

  blinkm.off();  // turn off the playing light script

  //blinkm.setAddress( blinkm_addr_default );  // uncomment to auto set address to default

  help();

  /*
  byte addr = blinkm.getAddress(blinkm_addr);
  if( addr != blinkm_addr ) {
    if( addr == -1 ) 
      Serial.println("\r\nerror: no response");
    else if( addr != blinkm_addr ) {
      Serial.print("\r\nerror: addr mismatch, addr received: ");
      Serial.println(addr, HEX);
    }
  }
  */
  Serial.print(F("cmd>"));
}

// arduino loop func
void loop()
{
  int32_t num;
  //read the serial port and create a string out of what you read
  if( readSerialString() ) {
    Serial.println(serInStr);
    char cmd = serInStr[0];  // first char is command

    // argument parsing
    char* str = serInStr;
    while( *++str == ' ' );  // go past any intervening whitespace
    if( strlen(str) >= 6 ) { // 6 chars in a hex color code
      str[6] = '\0';
      num = strtol(str, NULL, 16); // parse color
    } else { 
      num = atoi(str);
    }

    // command parsing
    if( cmd == '?' ) {
      help();
    }
    else if( cmd == 'c' || cmd=='h' || cmd == 'C' || cmd == 'H' ) {
      byte a = (num >> 16) & 0xff;
      byte b = (num >>  8) & 0xff;
      byte c = (num >>  0) & 0xff;

      if( cmd == 'c' ) {
        Serial.print("Fade to r,g,b:");
        blinkm.fadeToRGB( a,b,c );
      } 
      else if( cmd == 'h' ) {
        Serial.print("Fade to h,s,b:");
        blinkm.fadeToHSB( a,b,c );
      } 
      else if( cmd == 'C' ) {
        Serial.print("Random by r,g,b:");
        blinkm.fadeToRandomRGB( a,b,c );
      } 
      else if( cmd == 'H' ) {
        Serial.print("Random by h,s,b:");
        blinkm.fadeToRandomHSB( a,b,c );
      }
      Serial.print(a,HEX); Serial.print(",");
      Serial.print(b,HEX); Serial.print(",");
      Serial.print(c,HEX); Serial.println();
    }
    else if( cmd == 'f' ) {
      Serial.print("Set fade speed to:"); Serial.println(num,DEC);
      blinkm.setFadeSpeed(num);
    }
    else if( cmd == 't' ) {
      Serial.print("Set time adj:"); Serial.println(num,DEC);
      blinkm.setTimeAdj(num);
    }
    else if( cmd == 'p' ) {
      Serial.print("Play script #");
      Serial.println(num,DEC);
      blinkm.playScript(num,0,0 );
    }
    else if( cmd == 'o' ) {
      Serial.println("Stop script");
      blinkm.stopScript();
    }
    else if( cmd == 'g' ) {
      Serial.print("Current color: ");
      byte r,g,b;
      blinkm.getRGBColor(&r,&g,&b);
      Serial.print("r,g,b:"); Serial.print(r,HEX);
      Serial.print(",");      Serial.print(g,HEX);
      Serial.print(",");      Serial.println(b,HEX);
    }
    /*
      else if( cmd == 'W' ) { 
      Serial.println("Writing new eeprom script");
      for(int i=0; i<6; i++) {
      blinkm_script_line l = script_lines[i];
      blinkm.writeScriptLine( blinkm_addr, 0, i, l.dur,
      l.cmd[0],l.cmd[1],l.cmd[2],l.cmd[3]);
      }
      }
    */
    else if( cmd == 'A' ) {
      if( num>0 && num<0xff ) {
        Serial.print("Setting address to: ");
        Serial.println(num,DEC);
        blinkm.changeAddress(num);
        //blinkm_addr = num;
      } else if ( num == 0 ) {
        Serial.println("Resetting address to default 9: ");
        //blinkm_addr = 9;
        blinkm.changeAddress( blinkm_addr_default );
      }
    }
    else if( cmd == 'a' ) {
      Serial.print("Address: ");
      //num = blinkm.getAddress(0); 
      num = blinkm.getAddress(); 
      Serial.println(num);
    }
    else if( cmd == '@' ) {
      Serial.print("Will now talk on blinkm address: ");
      Serial.println(num,DEC);
      blinkm.talkTo( num );
    }
    else if( cmd == 'Z' ) { 
      Serial.print("blinkm version: ");
      num = blinkm.getVersion();
      if( num == -1 )
        Serial.println("couldn't get version");
      Serial.print( (char)(num>>8) ); 
      Serial.println( (char)(num&0xff) );
    }
    else if( cmd == 'B' ) {
      Serial.print("Set startup mode:"); Serial.println(num,DEC);
      blinkm.setStartupParams(num, 0,0,1,0);
    }
    else if( cmd == 'i' ) {
      Serial.print("get Inputs: ");
      byte inputs[4];
      blinkm.getInputs(inputs); 
      for( byte i=0; i<4; i++ ) {
        Serial.print(inputs[i],HEX);
        Serial.print( (i<3)?',':'\n');
      }
    }
    else if( cmd == 's' ) { 
      lookForBlinkM();
    }
    else if( cmd == 'S' ) {
      Serial.println("Scanning I2C bus from 1-100:");
      blinkm.ScanI2CBus(1,100, scanfunc);
      Serial.println();
    }
    else if( cmd == 'R' ) {
      Serial.println("Doing Factory Reset");
      blinkm.doFactoryReset();
    }
    else if( cmd == 'l' ) { 
        Serial.print("Set LEDn: ");
        Serial.println(num, BIN);
        blinkm.mk2setLED( num );
    }
    else if( cmd == 'r' ) { 
        Serial.print("rotate LEDs: ");
        Serial.println(num);
        blinkm.mk2rotateLEDs( num );
    }
    else if( cmd == '!' ) {
      Serial.print("");
    }
    else { 
      Serial.println("Unrecognized cmd");
    }
    serInStr[0] = 0;  // say we've used the string
    Serial.print("cmd>");
  } //if( readSerialString() )
  
}

// read a string from serial and store it in an array
// uses global variable serInStr
uint8_t readSerialString()
{
  Serial.setTimeout(100);
  memset( serInStr, 0, sizeof(serInStr) ); // set it all to zero
  int n = Serial.readBytesUntil( '\n', serInStr, serStrLen );
  return n;
}

// print help message
void help()
{
  for( int i=0; i<strlen(helpstr); i++ ) { 
    Serial.print( (char) pgm_read_byte(helpstr+i) );
  }
}
