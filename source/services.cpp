#include "MicroBit.h"
#include "MicroBitUARTService.h"
#include "communication.h"
#include "main.h"
#include "services.h"
#include "protocol.h"
#include <stdbool.h>

int beat = 500;
bool LEDSoSOn = false;
bool LEDSoSFinished = true;

bool SirenOn = false;
bool SirenFinished = true;

bool FanfareOn = false;
bool FanfareFinished = true;

bool PartyOn = false;
bool PartyFinished = true;

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
 * Function: processLEDRequest()
 *
 * Description: Determines the LED action to take
 *
 **********************************************************/
void processLedRequest(int serviceData) {
    if (serviceData == SERVICE_LED_ON) {

        // If the SoS function is running, wait for it to
        // finish its round.
        if (LEDSoSOn) {
            LEDSoSOn = false;
            while (!LEDSoSFinished)
                uBit.sleep(500);
        }

        // Switch on the power to the LED
        LED.setDigitalValue(1);

        // Send a notification back to the client
        sendAck(SERVICE_LED, SERVICE_LED_ON, SERVICE_ACK_OK);

        // Display the result on the LEDs
        uBit.display.scrollAsync("LED on");

    } else if (serviceData == SERVICE_LED_OFF) {
        // Switch off the power to the LED
        LED.setDigitalValue(0);

        // If the LED SoS is running, disable it
        LEDSoSOn = false;

        // Send a notification back to the client
        sendAck(SERVICE_LED, SERVICE_LED_OFF, SERVICE_ACK_OK);

        // Display the result on the LEDs
        uBit.display.scrollAsync("LED off");

    } else if (serviceData == SERVICE_LED_SOS) {
        // Perform SOS actions
        LEDSoSOn = true;

        create_fiber(LED_SOS);

        // Send a notification back to the client
        sendAck(SERVICE_LED, SERVICE_LED_SOS, SERVICE_ACK_OK);

        // Display the result on the LEDs
        uBit.display.scrollAsync("LED SOS");

    } else {
        // Send a notification back to the client
        sendAck(SERVICE_LED, SERVICE_UNKNOWN, SERVICE_ACK_FAILED);
    }
}
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
 * Function: processBuzzerRequest()
 *
 * Description: Determines the Buzzer action to take
 *
 **********************************************************/
void processBuzzerRequest(int serviceData) {

    if (serviceData == SERVICE_BUZZER_BASIC) {

        // If the Siren function is running, wait for it to
        // finish its round.
        if (SirenOn){
            SirenOn = false;
            while (!SirenFinished)
                uBit.sleep(500);
        } else if (FanfareOn){
            FanfareOn = false;
            while (!FanfareFinished)
                uBit.sleep(500);
        }


        // Send single note to the Buzzer
        BUZZER.setAnalogValue(511);     // square wave
        BUZZER.setAnalogPeriodUs(3823); // note C4 = freq 261.63Hz

        // Send a notification back to the client
        sendAck(SERVICE_BUZZER, SERVICE_BUZZER_BASIC, SERVICE_ACK_OK);

        // Display the result on the LEDs
        uBit.display.scrollAsync("Buzzer on");

    } else if (serviceData == SERVICE_BUZZER_OFF) {
        // Switch off the power to the Buzzer
        BUZZER.setDigitalValue(0);

        // If the Siren is running, disable it
        SirenOn = false;

        // Send a notification back to the client
        sendAck(SERVICE_BUZZER, SERVICE_BUZZER_OFF, SERVICE_ACK_OK);

        // Display the result on the LEDs
        uBit.display.scrollAsync("Buzzer off");

    } else if (serviceData == SERVICE_BUZZER_SIREN) {

        if (FanfareOn){
            FanfareOn = false;
            while (!FanfareFinished)
                uBit.sleep(500);
        }
        // Perform Siren actions
        SirenOn = true;

        create_fiber(buzzSiren);

        // Send a notification back to the client
        sendAck(SERVICE_BUZZER, SERVICE_BUZZER_SIREN, SERVICE_ACK_OK);

        // Display the result on the LEDs
        uBit.display.scrollAsync("Buzzer Siren");

    } else if (serviceData == SERVICE_BUZZER_FANFARE) {

        if (SirenOn){
            SirenOn = false;
            while (!SirenFinished)
                uBit.sleep(500);
        }

        FanfareOn = true;

        create_fiber(buzzFanfare);
//                                  }
        // Send a notification back to the client
        sendAck(SERVICE_BUZZER, SERVICE_BUZZER_FANFARE, SERVICE_ACK_OK);

        // Display the result on the LEDs
        uBit.display.scrollAsync("Buzzer Fanfare");

    } else {
        // Send a notification back to the client
        sendAck(SERVICE_BUZZER, SERVICE_UNKNOWN, SERVICE_ACK_FAILED);
    }
}
/***********************************************************
 *
 * Function: buzzSiren()
 *
 * Description: Makes a siren sound using the buzzer.
 *
 **********************************************************/
void buzzSiren(){

    SirenFinished = false;

    while (1) {
      if (SirenOn){
        BUZZER.setAnalogValue(511);     // square wave
        BUZZER.setAnalogPeriodUs(3823); // note C4 = freq 261.63Hz
        uBit.sleep(400);
        BUZZER.setAnalogValue(511);     // square wave
        BUZZER.setAnalogPeriodUs(5102); // note G3 = freq 196.00Hz
        uBit.sleep(400);
      }else{
        // Another fibre has terminated the Siren
        SirenFinished = true;
        release_fiber();
        break;
      }
    }


}

/***********************************************************
 *
 * Function: buzzFanfare()
 *
 * Description: Plays a tune using the buzzer.
 *
 **********************************************************/
void buzzFanfare(){

    FanfareFinished = false;

    while (1) {
      if (FanfareOn){

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
        }else{
          //Another fibre has terminated the Fanfare
          FanfareFinished = true;
          break;
        }
      }

}

/*****************************************************************************************************************
 * FAN Functions
 ****************************************************************************************************************/
/***********************************************************
 *
 * Function: processFanRequest()
 *
 * Description: Determines the Fan action to take
 *
 **********************************************************/
void processFanRequest(int serviceData) {

    if (serviceData == SERVICE_FAN_OFF) {
        // Switch off the power to the Fan
        FAN.setDigitalValue(0);

        // Send a notification back to the client
        sendAck(SERVICE_FAN, SERVICE_FAN_OFF, SERVICE_ACK_OK);

        // Display the result on the LEDs
        uBit.display.scrollAsync("Fan off");

    } else if (serviceData == SERVICE_FAN_SLOW) {
        // Set Fan power to ~25%
        FAN.setAnalogValue(255);

        // Send a notification back to the client
        sendAck(SERVICE_FAN, SERVICE_FAN_SLOW, SERVICE_ACK_OK);

        // Display the result on the LEDs
        uBit.display.scrollAsync("Fan setting: SLOW");

    } else if (serviceData == SERVICE_FAN_MED) {
        // Set Fan power to ~50%
        FAN.setAnalogValue(511);

        // Send a notification back to the client
        sendAck(SERVICE_FAN, SERVICE_FAN_MED, SERVICE_ACK_OK);

        // Display the result on the LEDs
        uBit.display.scrollAsync("Fan setting: MED");

    } else if (serviceData == SERVICE_FAN_FAST) {
        // Set Fan power to 100%
        FAN.setAnalogValue(1023);

        // Send a notification back to the client
        sendAck(SERVICE_FAN, SERVICE_FAN_FAST, SERVICE_ACK_OK);

        // Display the result on the LEDs
        uBit.display.scrollAsync("Fan setting: FAST");

    } else {
        // Unsupported Fan action.
        // Send a notification back to the client
        sendAck(SERVICE_FAN, SERVICE_UNKNOWN, SERVICE_ACK_FAILED);
    }
}

/*****************************************************************************************************************
 * RGB LED Functions
 ****************************************************************************************************************/
/***********************************************************
 *
 * Function: processRgbLedRequest()
 *
 * Description: Determines the RGB LED action to take
 *
 **********************************************************/
void processRgbLedRequest(int serviceData) {

    if (serviceData == SERVICE_RGB_OFF) {
        // Switch off the power to the Fan
        FAN.setDigitalValue(0);

        // If Party is running, end it
        PartyOn = false;

        // Send a notification back to the client
        sendAck(SERVICE_RGB_LED, SERVICE_RGB_OFF, SERVICE_ACK_OK);

        // Display the result on the LEDs
        uBit.display.scrollAsync("RGB LED off");

    } else if (serviceData == SERVICE_RGB_RED) {

        // If the Party function is running, wait for it to
        // end.
        if (PartyOn) {
          PartyOn = false;
          while (!PartyFinished)
              uBit.sleep(500);
        }

        // Set RGB LED to Red
        RGB_RED.setDigitalValue(1);
        RGB_GREEN.setDigitalValue(0);
        RGB_BLUE.setDigitalValue(0);

        // Send a notification back to the client
        sendAck(SERVICE_RGB_LED, SERVICE_RGB_RED, SERVICE_ACK_OK);

        // Display the result on the LEDs
        uBit.display.scrollAsync("RGB LED setting: Red");

    } else if (serviceData == SERVICE_RGB_BLUE) {

        // If the Party function is running, wait for it to
        // end.
        if (PartyOn) {
          PartyOn = false;
          while (!PartyFinished)
              uBit.sleep(500);
        }

        // Set RGB LED to Blue
        RGB_RED.setDigitalValue(0);
        RGB_GREEN.setDigitalValue(0);
        RGB_BLUE.setDigitalValue(1);

        // Send a notification back to the client
        sendAck(SERVICE_RGB_LED, SERVICE_RGB_BLUE, SERVICE_ACK_OK);

        // Display the result on the LEDs
        uBit.display.scrollAsync("RGB LED setting: Blue");

    } else if (serviceData == SERVICE_RGB_GREEN) {

        // If the Party function is running, wait for it to
        // end.
        if (PartyOn) {
          PartyOn = false;
          while (!PartyFinished)
              uBit.sleep(500);
        }

        // Set RGB LED to Green
        RGB_RED.setDigitalValue(0);
        RGB_GREEN.setDigitalValue(1);
        RGB_BLUE.setDigitalValue(0);

        // Send a notification back to the client
        sendAck(SERVICE_RGB_LED, SERVICE_RGB_GREEN, SERVICE_ACK_OK);

        // Display the result on the LEDs
        uBit.display.scrollAsync("RGB LED setting: Green");

    } else if (serviceData == SERVICE_RGB_MAGENTA) {

        // If the Party function is running, wait for it to
        // end.
        if (PartyOn) {
          PartyOn = false;
          while (!PartyFinished)
              uBit.sleep(500);
        }

        // Set RGB LED to Magenta
        RGB_RED.setDigitalValue(1);
        RGB_GREEN.setDigitalValue(0);
        RGB_BLUE.setDigitalValue(1);

        // Send a notification back to the client
        sendAck(SERVICE_RGB_LED, SERVICE_RGB_MAGENTA, SERVICE_ACK_OK);

        // Display the result on the LEDs
        uBit.display.scrollAsync("RGB LED setting: Magenta");

    } else if (serviceData == SERVICE_RGB_YELLOW) {

        // If the Party function is running, wait for it to
        // end.
        if (PartyOn) {
          PartyOn = false;
          while (!PartyFinished)
              uBit.sleep(500);
        }

        // Set RGB LED to Yellow
        RGB_RED.setDigitalValue(1);
        RGB_GREEN.setDigitalValue(1);
        RGB_BLUE.setDigitalValue(0);

        // Send a notification back to the client
        sendAck(SERVICE_RGB_LED, SERVICE_RGB_YELLOW, SERVICE_ACK_OK);

        // Display the result on the LEDs
        uBit.display.scrollAsync("RGB LED setting: Yellow");

    } else if (serviceData == SERVICE_RGB_CYAN) {

        // If the Party function is running, wait for it to
        // end.
        if (PartyOn) {
          PartyOn = false;
          while (!PartyFinished)
              uBit.sleep(500);
        }

        // Set RGB LED to Cyan
        RGB_RED.setDigitalValue(0);
        RGB_GREEN.setDigitalValue(1);
        RGB_BLUE.setDigitalValue(1);

        // Send a notification back to the client
        sendAck(SERVICE_RGB_LED, SERVICE_RGB_CYAN, SERVICE_ACK_OK);

        // Display the result on the LEDs
        uBit.display.scrollAsync("RGB LED setting: Cyan");

    } else if (serviceData == SERVICE_RGB_WHITE) {

        // If the Party function is running, wait for it to
        // end.
        if (PartyOn) {
          PartyOn = false;
          while (!PartyFinished)
              uBit.sleep(500);
        }

        // Set RGB LED to White
        RGB_RED.setDigitalValue(1);
        RGB_GREEN.setDigitalValue(1);
        RGB_BLUE.setDigitalValue(1);

        // Send a notification back to the client
        sendAck(SERVICE_RGB_LED, SERVICE_RGB_WHITE, SERVICE_ACK_OK);

        // Display the result on the LEDs
        uBit.display.scrollAsync("RGB LED setting: White");

    } else if (serviceData == SERVICE_RGB_PARTY) {

        // Perform Party actions
        PartyOn = true;

        create_fiber(rgbParty);

        // Send a notification back to the client
        sendAck(SERVICE_RGB_LED, SERVICE_RGB_PARTY, SERVICE_ACK_OK);

        // Display the result on the LEDs
        uBit.display.scrollAsync("LET'S PARTY");

    } else {
        // Unsupported RGB LED action.
        // Send a notification back to the client
        sendAck(SERVICE_RGB_LED, SERVICE_UNKNOWN, SERVICE_ACK_FAILED);
    }
}

/***********************************************************
*
* Function: rgbParty()
*
* Description: Activates Party Mode
*
**********************************************************/
void rgbParty(){

    PartyFinished = false;

    while(1){
      if (PartyOn){
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
      }else{
        // Another fibre has ended the party. :(
        PartyFinished = true;
        release_fiber();
        break;
      }
    }

}
