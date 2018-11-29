//
// Created by jon on 29/11/18.
//
#include "MicroBit.h"
#include "MicroBitUARTService.h"
#include "main.h"
#include "services.h"
#include <stdbool.h>

int beat = 500;

/***********************************************************
 *
 * Function: flashScreen()
 *
 * Description: Flashes the Microbit display three times
 *
 **********************************************************/
void flashScreen(){

    uBit.display.clear();
    uBit.sleep(200);
    uBit.display.printChar(digit[0]);
    uBit.sleep(200);
    uBit.display.clear();
    uBit.sleep(200);
}

/*****************************************************************************************************************
 * LED Functions
 ****************************************************************************************************************/

/***********************************************************
 *
 * Function: LED_SOS()
 *
 * Description: Sends an SOS signal using the LED
 *
 **********************************************************/
void LED_SOS(){

    LEDSoSFinished = false;

    while(1) {
        if (LEDSoSOn) {

            LED.setDigitalValue(1);
            uBit.sleep(100);
            LED.setDigitalValue(0);
            uBit.sleep(100);
            LED.setDigitalValue(1);
            uBit.sleep(100);
            LED.setDigitalValue(0);
            uBit.sleep(100);
            LED.setDigitalValue(1);
            uBit.sleep(100);
            LED.setDigitalValue(0);
            uBit.sleep(1000);

            LED.setDigitalValue(1);
            uBit.sleep(500);
            LED.setDigitalValue(0);
            uBit.sleep(500);
            LED.setDigitalValue(1);
            uBit.sleep(500);
            LED.setDigitalValue(0);
            uBit.sleep(500);
            LED.setDigitalValue(1);
            uBit.sleep(500);
            LED.setDigitalValue(0);
            uBit.sleep(1000);

            LED.setDigitalValue(1);
            uBit.sleep(100);
            LED.setDigitalValue(0);
            uBit.sleep(100);
            LED.setDigitalValue(1);
            uBit.sleep(100);
            LED.setDigitalValue(0);
            uBit.sleep(100);
            LED.setDigitalValue(1);
            uBit.sleep(100);
            LED.setDigitalValue(0);
            uBit.sleep(1000);

            uBit.sleep(1000);
        } else {
            // Another fibre has switched off the LED. Quit.
            LEDSoSFinished = true;
            release_fiber();
            break;
        }

    }
}

/*****************************************************************************************************************
 * Buzzer Functions
 ****************************************************************************************************************/

/***********************************************************
 *
 * Function: buzzSiren()
 *
 * Description: Makes a siren sound using the buzzer.
 *
 **********************************************************/
void buzzSiren(){
    BUZZER.setAnalogValue(511);     // square wave
    BUZZER.setAnalogPeriodUs(3823); // note C4 = freq 261.63Hz
    uBit.sleep(400);
    BUZZER.setAnalogValue(511);     // square wave
    BUZZER.setAnalogPeriodUs(5102); // note G3 = freq 196.00Hz
    uBit.sleep(400);
}

/***********************************************************
 *
 * Function: buzzFanfare()
 *
 * Description: Plays a tune using the buzzer.
 *
 **********************************************************/
void buzzFanfare(){

    BUZZER.setAnalogValue(511);     // square wave
    BUZZER.setAnalogPeriodUs(3823); // note C4 = freq 261.63Hz
    uBit.sleep(beat);
    BUZZER.setAnalogValue(511);     // square wave
    BUZZER.setAnalogPeriodUs(3033); // note E4 = freq 329.63Hz
    uBit.sleep(beat);
    BUZZER.setAnalogValue(511);     // square wave
    BUZZER.setAnalogPeriodUs(3405); // note D4 = freq 293.66Hz
    uBit.sleep(beat);
    BUZZER.setAnalogValue(511);     // square wave
    BUZZER.setAnalogPeriodUs(3823); // note C4 = freq 261.63Hz
    uBit.sleep(beat);
    BUZZER.setAnalogValue(511);     // square wave
    BUZZER.setAnalogPeriodUs(2551); // note G4 = freq 392.00Hz
    uBit.sleep(beat);
    BUZZER.setAnalogValue(511);     // square wave
    BUZZER.setAnalogPeriodUs(2863); // note F4 = freq 349.23Hz
    uBit.sleep(beat);
    BUZZER.setAnalogValue(511);     // square wave
    BUZZER.setAnalogPeriodUs(3033); // note E4 = freq 329.63Hz
    uBit.sleep(beat);
    BUZZER.setAnalogValue(511);     // square wave
    BUZZER.setAnalogPeriodUs(1911); // note C5 = freq 523.25Hz
    uBit.sleep(beat);
    BUZZER.setAnalogValue(511);     // square wave
    BUZZER.setAnalogPeriodUs(1911); // note C5 = freq 523.25Hz
    uBit.sleep(beat);
    BUZZER.setAnalogValue(511);     // square wave
    BUZZER.setAnalogPeriodUs(2273); // note A4 = freq 440.00Hz
    uBit.sleep(beat/2);
    BUZZER.setAnalogValue(511);     // square wave
    BUZZER.setAnalogPeriodUs(2025); // note B4 = freq 493.88Hz
    uBit.sleep(beat/2);
    BUZZER.setAnalogValue(511);     // square wave
    BUZZER.setAnalogPeriodUs(1911); // note C5 = freq 523.25Hz
    uBit.sleep(2*beat);
    BUZZER.setAnalogValue(511);     // square wave
    BUZZER.setAnalogPeriodUs(2273); // note A4 = freq 440.00Hz
    uBit.sleep(beat);
    BUZZER.setAnalogValue(511);     // square wave
    BUZZER.setAnalogPeriodUs(2551); // note G4 = freq 392.00Hz
    uBit.sleep(beat);
    BUZZER.setAnalogValue(511);     // square wave
    BUZZER.setAnalogPeriodUs(3033); // note E4 = freq 329.63Hz
    uBit.sleep(beat);
    BUZZER.setAnalogValue(511);     // square wave
    BUZZER.setAnalogPeriodUs(3823); // note C4 = freq 261.63Hz
    uBit.sleep(beat);
    BUZZER.setAnalogValue(511);     // square wave
    BUZZER.setAnalogPeriodUs(3823); // note C4 = freq 261.63Hz
    uBit.sleep(beat);
    BUZZER.setAnalogValue(511);     // square wave
    BUZZER.setAnalogPeriodUs(4049); // note B3 = freq 246.94Hz
    uBit.sleep(beat);
    BUZZER.setAnalogValue(511);     // square wave
    BUZZER.setAnalogPeriodUs(3823); // note C4 = freq 261.63Hz
    uBit.sleep(beat);
    BUZZER.setAnalogValue(511);     // square wave
    BUZZER.setAnalogPeriodUs(3033); // note E4 = freq 329.63Hz
    uBit.sleep(beat);
    BUZZER.setAnalogValue(511);     // square wave
    BUZZER.setAnalogPeriodUs(3405); // note D4 = freq 293.66Hz
    uBit.sleep(beat);
    BUZZER.setAnalogValue(511);     // square wave
    BUZZER.setAnalogPeriodUs(3405); // note D4 = freq 293.66Hz
    uBit.sleep(beat);
    BUZZER.setAnalogValue(511);     // square wave
    BUZZER.setAnalogPeriodUs(3405); // note D4 = freq 293.66Hz
    uBit.sleep(beat);
    BUZZER.setDigitalValue(0);

}

/*****************************************************************************************************************
 * RGB LED Functions
 ****************************************************************************************************************/

/***********************************************************
*
* Function: rgbParty()
*
* Description: Activates Party Mode
*
**********************************************************/
void rgbParty(){
    RGB_RED.setDigitalValue(1);   //Red
    RGB_GREEN.setDigitalValue(0);
    RGB_BLUE.setDigitalValue(0);
    uBit.sleep(3000);
    RGB_GREEN.setDigitalValue(1); //Yellow
    uBit.sleep(300);
    RGB_RED.setDigitalValue(0);   //Green
    uBit.sleep(300);
    RGB_BLUE.setDigitalValue(1);  //Cyan
    uBit.sleep(300);
    RGB_GREEN.setDigitalValue(0); //Blue
    uBit.sleep(300);
    RGB_RED.setDigitalValue(1);   //Magenta
    uBit.sleep(300);
    RGB_GREEN.setDigitalValue(1); //White
    uBit.sleep(300);
}
