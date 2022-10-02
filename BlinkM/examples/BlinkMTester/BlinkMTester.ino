/*
 * BlinkMTester -- Simple "command-line" tool to play with BlinkMs
 *
 *  Once you load this sketch on to your Arduino, open the Serial Monitor
 *  and you'll see a menu of things to do.
 *
 * 2007-20, Tod E. Kurt, ThingM, http://thingm.com/
 *
 *
 * BlinkM connections to Arduino Uno
 * PWR - -- gnd -- black -- Gnd
 * PWR + -- +5V -- red   -- 5V
 * I2C d -- SDA -- green -- Analog In 4
 * I2C c -- SCL -- blue  -- Analog In 5
 *
 * Note: This sketch DOES NOT reset the I2C address of the BlinkM.
 *       If you want to change the I2C address use the 'A<n>' command.
 *
 * Wiring hookups:
 *
 * Arduino Uno to BlinkM:
 * Gnd - Gnd, 5V - Vcc, A4 - SDA, A5 - SCL
 *
 * Sparkfun Pro Micro (ATmega32U4) to BlinkM:
 * Gnd - Gnd, 5V - Vcc, D2 - SDA, D3 - SCL
 *
 * Adafruit Trinket M0 to BlinkM2:
 * Gnd - Gnd, 3V - Vcc, D0 - SDA, D2 - SCL
 *
 *
 * Can upload to Arduino via commandline with:
 *  arduino-cli compile --fqbn arduino:avr:micro BlinkMTester  (say for Arduino Pro Micro)
 *  arduino-cli -v upload -p /dev/tty.usbmodem141301 --fqbn arduino:avr:micro BlinkMTester
 *
 * BlinkM2 feature ideas:
 * - expand algorithmic pattern generation (lots of work on this already)
 * - low-power sleep mode
 * - wake from sleep on input
 *
 */


#include "BlinkM.h"
#include "Wire.h"

#include <avr/pgmspace.h>  // for progmem stuff
#include <stdlib.h>

#define BLINKM2_CMDS

// set this if you're plugging a BlinkM directly into an Arduino,
// into the standard position on analog in pins 2,3,4,5
// otherwise you can set it to false or just leave it alone
const boolean BLINKM_ARDUINO_POWERED = false;

const byte blinkm_addr_default = 0x09; // the default address of all BlinkMs

BlinkM blinkm = BlinkM();

const int serStrLen = 30;
String serInStr = String(serStrLen);


// print help message
void help()
{
    Serial.println( F(
  "\nBlinkMTester!\n"
  "'c<rrggbb>'  fade to an rgb color\n"
  "'h<hhssbb>'  fade to an hsb color\n"
  "'C<rrggbb>'  fade to a random rgb color\n"
  "'H<hhssbb>'  fade to a random hsb color\n"
  "'p<n>'       play a script\n"
  "'o'          stop currrently playing script\n"
  "'O'          stop script and turn off BlinkM\n"
  "'f<nn>'      change fade speed\n"
  "'t<nn>'      set time adj\n"
  "'g'          get current color\n"
  "'a'          get I2C address\n"
  "'A<n>'       set I2C address\n"
  "'Z'          get BlinkM version\n"
  "'i'          get input values\n"
  "'B'          set startup params to default\n"
  "'R<nn>'      read EEPROM script line\n"
  "'W'          write EEPROM scirpt line\n"
  "'b'          set brightness (v2)\n"
  "'.'          set which LED to act on (0=all) (v2)\n"
  "'%'          return free RAM (v2)\n"
  "'s'/'S'      scan i2c bus for 1st BlinkM / search for devices\n"
  "'@<nn>'      select which BlinkM addr to talk on\n"
  "'!'          return BlinkM to factory settings\n"
  "'?'          for this help msg\n\n"
                      ));
}

// called when address is found in BlinkM_scanI2CBus()
void scanfunc( byte addr, byte result )
{
    Serial.print(F("addr: "));
    Serial.print(addr,DEC);
    Serial.print( (result==0) ? F(" found!"):F("       "));
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
  Serial.begin(115200);
  Serial.println("\nBlinkMTester!\n");

  if( BLINKM_ARDUINO_POWERED ) {
    blinkm.powerUp();
  }

  blinkm.begin(true);

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
  printCmdPrompt();
}

//
void printCmdPrompt()
{
    Serial.print("BlinkM addr ");
    Serial.print(blinkm.getTalkToAddress());
    Serial.print(F(": cmd> "));
}


//
// if 6-digit color, assume hex, otherwise any num
int str_to_nums(int32_t* buffer, char *str, int buflen)
{
    char    *s;
    int     pos = 0;
    while((s = strtok(str, ", ")) != NULL && pos < buflen){
        str = NULL;
        buffer[pos++] = strtol(s, NULL, strlen(s)==6 ? 16 : 0); // parse as hexstring
    }
    return pos;
}

// arduino loop func
void loop()
{
  int32_t num;

  if( !Serial.available() ) { return; }
  serInStr = Serial.readStringUntil('\n');
  serInStr.trim();
  Serial.println(serInStr);

  char cmd = serInStr[0];  // first char is command

  // argument parsing

  const char* str = serInStr.c_str();
  char str_work[40];
  strcpy(str_work, str+1);
  int32_t nums[8];
  int n = str_to_nums( nums, str_work, 8);

  num = nums[0]; // by default, assuming we got a hex string
  if( n > 1 )  {  // we parsed more than a single number, so make it a single number
      num = (nums[0] << 16 ) | (nums[1] << 8) | (nums[0] << 0);
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
          Serial.print(F("Fade to r,g,b:"));
          blinkm.fadeToRGB( a,b,c );
      }
      else if( cmd == 'h' ) {
          Serial.print(F("Fade to h,s,b:"));
          blinkm.fadeToHSB( a,b,c );
      }
      else if( cmd == 'C' ) {
          Serial.print(F("Random by r,g,b:"));
          blinkm.fadeToRandomRGB( a,b,c );
      }
      else if( cmd == 'H' ) {
          Serial.print(F("Random by h,s,b:"));
          blinkm.fadeToRandomHSB( a,b,c );
      }
      Serial.print(a,HEX); Serial.print(",");
      Serial.print(b,HEX); Serial.print(",");
      Serial.print(c,HEX); Serial.println();
  }
  else if( cmd == 'f' ) {
      Serial.print(F("Set fade speed to:")); Serial.println(num,DEC);
      blinkm.setFadeSpeed(num);
  }
  else if( cmd == 't' ) {
      Serial.print(F("Set time adj:")); Serial.println(num,DEC);
      blinkm.setTimeAdj(num);
  }
  else if( cmd == 'p' ) {
      Serial.print(F("Play script #"));
      Serial.println(num,DEC);
      blinkm.playScript(num,0,0 );
  }
  else if( cmd == 'o' ) {
      Serial.println(F("Stop script"));
      blinkm.stopScript();
  }
  else if( cmd == 'O' ) {
      Serial.println(F("Stop script & turn off"));
      blinkm.stopScript();
      blinkm.fadeToRGB( 0,0,0 );
  }
  else if( cmd == 'g' ) {
      Serial.print(F("Current color: "));
      byte r,g,b;
      blinkm.getRGBColor(&r,&g,&b);
      Serial.print("r,g,b:"); Serial.print(r,HEX);
      Serial.print(",");      Serial.print(g,HEX);
      Serial.print(",");      Serial.println(b,HEX);
  }
  else if( cmd == 'R' ) {
      Serial.print(F("Read script line: ")); Serial.println(num,DEC);
      blinkm_script_line line;
      blinkm.readScriptLine(0, num, &line);
      Serial.print(F("  scriptline: duration:")); Serial.print(line.dur);
      Serial.print(F(" cmd:")); Serial.print((char)line.cmd[0]);
      for( int i=0; i<4; i++) {
          Serial.print(','); Serial.print(line.cmd[i]);
      }
      Serial.println();
  }
  else if( cmd == 'W' ) {
      Serial.println(F("Writing new eeprom script"));
      /* for(int i=0; i<6; i++) { */
      /*     blinkm_script_line l = script_lines[i]; */
      /*     blinkm.writeScriptLine( blinkm_addr, 0, i, l.dur, */
      /*                             l.cmd[0],l.cmd[1],l.cmd[2],l.cmd[3]); */
      /* } */
  }
  else if( cmd == 'A' ) {
      if( num>0 && num<0xff ) {
          Serial.print(F("Setting address to: "));
          Serial.println(num,DEC);
          blinkm.changeAddress(num);
          //blinkm_addr = num;
      } else if ( num == 0 ) {
          Serial.println(F("Resetting address to default 9: "));
          //blinkm_addr = 9;
          blinkm.changeAddress( blinkm_addr_default );
      }
  }
  else if( cmd == 'a' ) {
      Serial.print(F("Address: "));
      //num = blinkm.getAddress(0);
      num = blinkm.getAddress();
      Serial.println(num);
  }
  else if( cmd == '@' ) {
      Serial.print(F("Will now talk on blinkm address: "));
      Serial.println(num,DEC);
      blinkm.talkTo( num );
  }
  else if( cmd == 'Z' ) {
      Serial.print(F("BlinkM version: "));
      num = blinkm.getVersion();
      if( num == -1 )
          Serial.println(F("couldn't get version"));
      Serial.print( (char)(num>>8) );
      Serial.println( (char)(num&0xff) );
  }
  else if( cmd == 'B' ) {
      Serial.print(F("Set startup mode:")); Serial.println(num,DEC);
      blinkm.setStartupParams(num, 0,0,1,0);
  }
  else if( cmd == 'i' ) {
      Serial.print(F("get Inputs: "));
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
      Serial.println(F("Scanning I2C bus from 1-100:"));
      blinkm.ScanI2CBus(1,100, scanfunc);
      Serial.println();
  }
  else if( cmd == '!' ) {
      Serial.println(F("Doing Factory Reset"));
      blinkm.doFactoryReset();
  }
  else if( cmd == '.' ) {
      Serial.print(F("Set LEDn: "));
      Serial.println(num, BIN);
      blinkm.setLEDn( num );
  }
  else if( cmd == 'b' ) { // set brightness BLINKM2
      Serial.print(F("Set brightness: "));
      Serial.println(num);
      blinkm.setBrightness(num);
  }
  else if( cmd == '%' ) {  // free memory  BLINKM2
      Serial.print("Free RAM: ");
      uint8_t addr = blinkm.getAddress();
      Wire.beginTransmission(addr);
      Wire.write('%');
      Wire.endTransmission();
      Wire.requestFrom(addr, (byte)2);
      while( Wire.available() < 2 ) ;
      num |= (Wire.read() << 0);
      num |= (Wire.read() << 8);
      Serial.println(num);
  }

  else {
      Serial.print(F("Unrecognized cmd '")); Serial.print(serInStr); Serial.println("'");
  }
  //serInStr[0] = 0;  // say we've used the string

  printCmdPrompt();

}
