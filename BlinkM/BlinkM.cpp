
//
// ------------------------------------------------------
// BlinkM functions
// ------------------------------------------------------
//

#include "BlinkM.h"

#include <stdlib.h>

extern "C" { 
#include "utility/twi.h"  // from Wire library, so we can do bus scanning
}

//
BlinkMClass::BlinkMClass() : addr(0x09), gndPin(A2), pwrPin(A3)
{
}

//
// each call to twi_writeTo() should return 0 if device is there
// or other value (usually 2) if nothing is at that address
// 
void BlinkMClass::ScanI2CBus(byte from, byte to, 
                              void(*callback)(byte add, byte result) ) 
{
  byte rc;
  byte data = 0; // not used, just an address to feed to twi_writeTo()
  for( byte addr = from; addr <= to; addr++ ) {
    rc = twi_writeTo(addr, &data, 0, 1, 0 );
    callback( addr, rc );
  }
}

//
//
int8_t BlinkMClass::FindFirstI2CDevice(void)
{
  byte rc;
  byte data = 0; // not used, just an address to feed to twi_writeTo()
  for( byte addr=1; addr < 120; addr++ ) {  // only scan addrs 1-120
    rc = twi_writeTo(addr, &data, 0, 1, 0);
    if( rc == 0 ) return addr; // found an address
  }
  return -1; // no device found in range given
}

// call this before begin() if BlinkM is being powered by Arduino pins
void BlinkMClass::powerUp(void)
{
    pinMode( gndPin, OUTPUT );
    pinMode( pwrPin, OUTPUT );
    digitalWrite( gndPin, LOW );
    digitalWrite( pwrPin, HIGH );
    delay(100);         // wait for things to stabilize  
}

// 
void BlinkMClass::setPowerPins(byte pwrpin, byte gndpin)
{
    pwrPin = pwrpin;
    gndPin = gndpin;
}

// Call this first before calling any of the methods below
void BlinkMClass::begin(byte address)
{
    //pinMode(SDA, INPUT_PULLUP );
    //pinMode(SCL, INPUT_PULLUP );
    Wire.begin();                // join i2c bus (address optional for master)
    addr = address;
}

// point at a different BlinkM
void BlinkMClass::talkTo(byte address)
{
    addr = address;
}

// Sets the I2C address of the BlinkM.
// Uses "general call" broadcast address
void BlinkMClass::setAddress(byte newaddress)
{
    addr = newaddress;

    Wire.beginTransmission(0x00);  // general call (broadcast address)
    Wire.write('A');
    Wire.write(newaddress);
    Wire.write(0xD0);
    Wire.write(0x0D);  // dood!
    Wire.write(newaddress);
    Wire.endTransmission();
    delay(10);  // wait for it to write to EEPROM
}

// Gets the I2C address of the BlinKM
// Kind of redundant when sent to a specific address
// but uses to verify BlinkM communication
int BlinkMClass::getAddress()
{
    Wire.beginTransmission(0); //addr);  // FIXME?
    Wire.write('a');
    Wire.endTransmission();
    Wire.requestFrom(addr, (byte)1);
    if( Wire.available() ) {
        byte b = Wire.read();
        return b; 
    }
    return -1;
}

// Demonstrates how to verify you're talking to a BlinkM 
// and that you know its address
// Might be more useful to do this check in the caller
int BlinkMClass::checkAddress()
{
    int b = getAddress();
    if( b==-1 )
        return -1; // no response
    if( b == addr )
        return 0;  // address match
    else
        return 1;  // address mismatch
}

// Gets the BlinkM firmware version
int BlinkMClass::getVersion()
{
    Wire.beginTransmission(addr);
    Wire.write('Z');
    Wire.endTransmission();
    Wire.requestFrom(addr, (byte)2);
    if( Wire.available() ) {
        byte major_ver = Wire.read();
        byte minor_ver = Wire.read();
        return (major_ver<<8) + minor_ver; // FIXME: this is kinda dumb
    }
    return -1;
}

// sends a generic command of length 'cmdlen'
void BlinkMClass::sendCmd(byte* cmd, int cmdlen)
{
    Wire.beginTransmission(addr);
    for( byte i=0; i<cmdlen; i++) 
        Wire.write(cmd[i]);
    Wire.endTransmission();
}

// receives generic data
// returns 0 on success, and -1 if no data available
// note: responsiblity of caller to know how many bytes to expect
// FIXME: this doesn't wait for the correct number of bytes, 
//        so may fail in weird ways
int BlinkMClass::receiveBytes(byte* resp, byte len)
{
    Wire.requestFrom(addr, len);
    if( Wire.available() ) {
        for( int i=0; i<len; i++) 
            resp[i] = Wire.read();
        return 0;
    }
    return -1;
}


// Sets the speed of fading between colors.  
// Higher numbers means faster fading, 255 == instantaneous fading
void BlinkMClass::setFadeSpeed(byte fadespeed)
{
    Wire.beginTransmission(addr);
    Wire.write('f');
    Wire.write(fadespeed);
    Wire.endTransmission();  
}

// Sets the light script playback time adjust
// The timeadj argument is signed, and is an additive value to all
// durations in a light script. Set to zero to turn off time adjust.
void BlinkMClass::setTimeAdj(byte timeadj)
{
    Wire.beginTransmission(addr);
    Wire.write('t');
    Wire.write(timeadj);
    Wire.endTransmission();  
}

// Fades to an RGB color
void BlinkMClass::fadeToRGB(byte red, byte grn, byte blu)
{
    Wire.beginTransmission(addr);
    Wire.write('c');
    Wire.write(red);
    Wire.write(grn);
    Wire.write(blu);
    Wire.endTransmission();
}

// Fades to an HSB color
void BlinkMClass::fadeToHSB(byte hue, byte sat, byte bri)
{
    Wire.beginTransmission(addr);
    Wire.write('h');
    Wire.write(hue);
    Wire.write(sat);
    Wire.write(bri);
    Wire.endTransmission();
}

// Sets an RGB color immediately
void BlinkMClass::setRGB(byte red, byte grn, byte blu)
{
    Wire.beginTransmission(addr);
    Wire.write('n');
    Wire.write(red);
    Wire.write(grn);
    Wire.write(blu);
    Wire.endTransmission();
}

// Fades to a random RGB color
void BlinkMClass::fadeToRandomRGB(byte rrnd, byte grnd, byte brnd)
{
    Wire.beginTransmission(addr);
    Wire.write('C');
    Wire.write(rrnd);
    Wire.write(grnd);
    Wire.write(brnd);
    Wire.endTransmission();
}

// Fades to a random HSB color
void BlinkMClass::fadeToRandomHSB(byte hrnd, byte srnd, byte brnd)
{
    Wire.beginTransmission(addr);
    Wire.write('H');
    Wire.write(hrnd);
    Wire.write(srnd);
    Wire.write(brnd);
    Wire.endTransmission();
}

// Gets current RGB value being displayed
void BlinkMClass::getRGBColor(byte* r, byte* g, byte* b)
{
    Wire.beginTransmission(addr);
    Wire.write('g');
    Wire.endTransmission();
    Wire.requestFrom(addr, (byte)3);
    while( Wire.available() < 3 ) ;  // FIXME:
    //if( Wire.available() )  { 
        *r = Wire.read();
        *g = Wire.read();
        *b = Wire.read();
        //}
}

// Plays light script 'script_id' on the BlinkM, 
// at position 'pos', repeating 'reps' times
void BlinkMClass::playScript(byte script_id, byte reps, byte pos)
{
    Wire.beginTransmission(addr);
    Wire.write('p');
    Wire.write(script_id);
    Wire.write(reps);
    Wire.write(pos);
    Wire.endTransmission();
}

// Stops any script currently playing on the BlinkM
void BlinkMClass::stopScript(void)
{
    Wire.beginTransmission(addr);
    Wire.write('o');
    Wire.endTransmission();
}

// Stops a blinkm from playing a script, resets fadespeed, sets color to black
void BlinkMClass::off(void)
{
    stopScript();
    setFadeSpeed(10);
    setRGB( 0,0,0 );
}

// Sets the script length and repeats of the given script_id
// (currently only script_id 0 is supported for writing)
void BlinkMClass::setScriptLengthReps(byte script_id, byte len, byte reps)
{
    Wire.beginTransmission(addr);
    Wire.write('l');
    Wire.write(script_id);
    Wire.write(len);
    Wire.write(reps);
    Wire.endTransmission();
}

// Fill up script_line with data from a script line
// currently only script_id = 0 works (eeprom script)
void BlinkMClass::readScriptLine(byte script_id, 
                                 byte pos, blinkm_script_line* script_line)
{
    Wire.beginTransmission(addr);
    Wire.write('R');
    Wire.write(script_id);
    Wire.write(pos);
    Wire.endTransmission();
    Wire.requestFrom(addr, (byte)5);
    while( Wire.available() < 5 ) ; // FIXME: wait until we get 7 bytes
    script_line->dur    = Wire.read();
    script_line->cmd[0] = Wire.read();
    script_line->cmd[1] = Wire.read();
    script_line->cmd[2] = Wire.read();
    script_line->cmd[3] = Wire.read();
}

// (currently only script_id 0 is supported for writing)
void BlinkMClass::writeScriptLine(byte script_id, 
                                  byte pos, byte dur,
                                  byte cmd, byte arg1, byte arg2, byte arg3)
{
//#ifdef BLINKM_FUNCS_DEBUG
    //Serial.print("writing line:");  Serial.print(pos,DEC);
    //Serial.print(" with cmd:"); Serial.print(cmd); 
    //Serial.print(" arg1:"); Serial.println(arg1,HEX);
//#endif
    Wire.beginTransmission(addr);
    Wire.write('w');
    Wire.write(script_id);
    Wire.write(pos);
    Wire.write(dur);
    Wire.write(cmd);
    Wire.write(arg1);
    Wire.write(arg2);
    Wire.write(arg3);
    Wire.endTransmission();
}


void BlinkMClass::writeScript(byte script_id, 
                        byte len, byte reps,
                        blinkm_script_line* lines)
{
//#ifdef BLINKM_FUNCS_DEBUG
//Serial.print("writing script to addr:"); Serial.print(addr,DEC);
//Serial.print(", script_id:"); Serial.println(script_id,DEC);
//#endif
    for(byte i=0; i < len; i++) {
        blinkm_script_line l = lines[i];
        writeScriptLine( script_id, i, l.dur,
                         l.cmd[0], l.cmd[1], l.cmd[2], l.cmd[3]);
    }
    setScriptLengthReps(script_id, len, reps);
}

// Set the 'boot' parameters of a BlinkM
void BlinkMClass::setStartupParams(byte mode, byte script_id,
                              byte reps, byte fadespeed, byte timeadj)
{
    Wire.beginTransmission(addr);
    Wire.write('B');
    Wire.write(mode);
    Wire.write(script_id);
    Wire.write(reps);
    Wire.write(fadespeed);
    Wire.write(timeadj);
    Wire.endTransmission();
} 

// Gets inputs of the BlinkM
// stores them in passed in array
// returns -1 on failure
int BlinkMClass::getInputs(byte inputs[])
{
    Wire.beginTransmission(addr);
    Wire.write('i');
    Wire.endTransmission();
    Wire.requestFrom(addr, (byte)4);
    while( Wire.available() < 4 ) ; // FIXME: wait until we get 4 bytes
    
    inputs[0] = Wire.read();
    inputs[1] = Wire.read();
    inputs[2] = Wire.read();
    inputs[3] = Wire.read();

    return 0;
}

// FIXME: test
int BlinkMClass::doFactoryReset(void) 
{
    setAddress( 0x09 );

    delay(30);

    //setStartupParamsDefault( 0x09 );
    
    delay(30);

    //  the example script we're going to write
    blinkm_script_line script1_lines[] = {
        {  1,  {'f',   10,  00,  00}},  // set fade speed to 10
        { 100, {'c', 0xff,0xff,0xff}},  // white
        {  50, {'c', 0xff,0x00,0x00}},  // red
        {  50, {'c', 0x00,0xff,0x00}},  // green
        {  50, {'c', 0x00,0x00,0xff}},  // blue 
        {  50, {'c', 0x00,0x00,0x00}},  // off
    };
    int script1_len = 6;  // number of script lines above
    
    writeScript( 0, script1_len, 0, script1_lines);

    /*
    BlinkMScript script = new BlinkMScript();
    script.add( new BlinkMScriptLine(  1, 'f',   10,   0,   0) );
    script.add( new BlinkMScriptLine(100, 'c', 0xff,0xff,0xff) );
    script.add( new BlinkMScriptLine( 50, 'c', 0xff,0x00,0x00) );
    script.add( new BlinkMScriptLine( 50, 'c', 0x00,0xff,0x00) );
    script.add( new BlinkMScriptLine( 50, 'c', 0x00,0x00,0xff) );
    script.add( new BlinkMScriptLine( 50, 'c', 0x00,0x00,0x00) );
    for( int i=0; i< 49-6; i++ ) {  // FIXME:  make this length correct
        script.add( new BlinkMScriptLine( 0, 'c', 0,0,0 ) );
    }
    
    writeScript( addr, script);
    */
    return 0;
}

// -------------------------------------------------------------------------

//
void BlinkMClass::mk2setLED(byte ledn)
{
  Wire.beginTransmission(addr);
  Wire.write('l');
  Wire.write(ledn);
  Wire.endTransmission();
}

//
void BlinkMClass::mk2rotateLEDs(byte rot)
{
  Wire.beginTransmission(addr);
  Wire.write('r');
  Wire.write(rot);
  Wire.endTransmission();
}


// Preinstantiate Object
BlinkMClass BlinkM = BlinkMClass();
