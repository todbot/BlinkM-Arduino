/*
 * BlinkM.h-- Arduino library to control BlinkM family products
 * --------------
 *
 *
 * Note: original version of this started as "BlinkM_funcs.h" 
 *      located in https://code.google.com/p/blinkm-projects/
 *
 *
 * 2007-14, Tod E. Kurt, ThingM, http://thingm.com/
 *
 */

#ifndef BLINKM_H
#define BLINKM_H

#include <Arduino.h>
#include <Wire.h>  // must also include Wire.h in your sketch

#include <inttypes.h>

#ifndef byte
#define byte uint8_t
#endif

const int defaultPwrPin = A2;
const int defaultGndPin = A3;

typedef struct {
    byte dur;
    byte cmd[4];    // cmd,arg1,arg2,arg3
} blinkm_script_line;


class BlinkM
{
 private:
    byte addr;  // can only talk to one BlinkM at a time, use talkTo to change
    byte gndPin;
    byte pwrPin;

 public: 
    BlinkM(void);
    BlinkM( byte addr );
    void begin(void);
    void begin( byte addr );
    void talkTo(byte address);
    void setPowerPins(byte pwrpin, byte gndpin);
    void powerUp(void);
    void powerDown(void);
    
    static int8_t FindFirstI2CDevice(void);
    static void ScanI2CBus(byte from, byte to, 
                           void(*callback)(byte add, byte result) );

    void changeAddress(byte address);
    int getAddress(void);
    int checkAddress(void);
    int getVersion(void);

    void sendCmd(byte* cmd, int cmdlen);
    int receiveBytes(byte* resp, byte len);

    void setFadeSpeed(byte fadespeed);
    void setFadeSpeed(byte fadespeed, byte naddr);

    void setTimeAdj(byte timeadj);
    void setTimeAdj(byte timeadj, byte naddr);

    void fadeToRGB(byte red, byte grn, byte bri );
    void fadeToRGB(byte red, byte grn, byte bri, byte naddr );

    void fadeToHSB(byte hue, byte sat, byte bri );
    void fadeToHSB(byte hue, byte sat, byte bri, byte naddr );

    void setRGB(byte red, byte grn, byte blu);
    void setRGB(byte red, byte grn, byte blu, byte naddr);

    void getRGBColor(byte* red, byte* grn, byte* blu);

    void fadeToRandomRGB(byte rrnd, byte grnd, byte brnd);
    void fadeToRandomHSB(byte hrnd, byte srnd, byte brnd);


    void playScript(byte script_id, byte reps, byte pos);
    void playScript(byte script_id, byte reps, byte pos, byte naddr);

    void stopScript( void );
    void stopScript( byte naddr );

    void off(void);
    void off(byte naddr);

    void setScriptLengthReps(byte script_id, byte len, byte reps );
    void readScriptLine(byte script_id, byte pos, 
                        blinkm_script_line* script_line);
    void writeScriptLine(byte script_id, byte pos, byte dur,
                         byte cmd, byte arg1, byte arg2,byte arg3);
    void writeScript(byte script_id,  byte len, byte reps,
                     blinkm_script_line* lines);

    void setStartupParams(byte mode, byte script_id,
                          byte reps, byte fadespeed, byte timeadj);

    int getInputs(byte inputs[]);

    int doFactoryReset(void);

    void mk2setLED(byte ledn);
    void mk2rotateLEDs(byte rot);

};

//extern BlinkMClass BlinkM;

#endif
