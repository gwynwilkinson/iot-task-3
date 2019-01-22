#include <stdbool.h>
#include "MicroBit.h"
#include "MicroBitUARTService.h"
#include "aes.h"
#include "sha1.h"
#include "main.h"
#include "communication.h"
#include "utility.h"
#include "protocol.h"
#include "services.h"
#include "ccitt-crc.h"
#include <string.h>

MicroBit uBit;
MicroBitUARTService *uart;

/* Buzzer is connected to Pin 0 */
MicroBitPin BUZZER(MICROBIT_ID_IO_P0, MICROBIT_PIN_P0, PIN_CAPABILITY_ALL);

/* Fan motor is connected to Pin 1 */
MicroBitPin FAN(MICROBIT_ID_IO_P1, MICROBIT_PIN_P1, PIN_CAPABILITY_ALL);

/* Single colour LED is connected on Pin 16 */
MicroBitPin LED(MICROBIT_ID_IO_P16, MICROBIT_PIN_P16, PIN_CAPABILITY_DIGITAL);

/* RGB LED is connected on Pin 13 (Red), Pin 14 (Green) and Pin 15 (Blue) */
MicroBitPin RGB_RED(MICROBIT_ID_IO_P13, MICROBIT_PIN_P13, PIN_CAPABILITY_DIGITAL);
MicroBitPin RGB_GREEN(MICROBIT_ID_IO_P14, MICROBIT_PIN_P14, PIN_CAPABILITY_DIGITAL);
MicroBitPin RGB_BLUE(MICROBIT_ID_IO_P15, MICROBIT_PIN_P15, PIN_CAPABILITY_DIGITAL);

int connected = 0;

/* Arrays for encrypted UART messages and decrypted final message */
char uartBuffer[33];
char decodedAsciiMsg[33];

static char charlookup[] = "0123456789";

unsigned char charKey[] = "000";

bool fCollectDigit0;
bool fCollectDigit1;
bool fCollectDigit2;

bool fPINCollected;

unsigned int digit[3] = {0,0,0};

bool fButtonAWait = false;
bool fButtonBWait = false;

char perSessionSalt[6] = "";

/***********************************************************
 *
 * Function: readPIN()
 *
 * Description: Sets our read flags so that we can input
 *              the PIN using buttons A & B, and returns
 *              the result
 *
 **********************************************************/
void readPIN(char* PIN)
{
    // initialise all our flags - entering most significant digit first
    fCollectDigit0=true;
    fCollectDigit1=false;
    fCollectDigit2=false;
    fPINCollected = false;

    // Initialise the PIN digits
    digit[0] = 0;
    digit[1] = 0;
    digit[2] = 0;

    // Debug printout
    uBit.serial.send("Waiting for PIN entry\n");

    // prompt user for input
    uBit.display.print("Enter PIN");
    uBit.sleep(200);
    uBit.display.printChar(charlookup[digit[0]]);

    // this while loop spins while the user enters the PIN using buttons A & B
    while(!fPINCollected)
    {
        uBit.sleep(1000);
    }

    // translates entered digits into decimal integer and returns to caller
    PIN[0] = charKey[0];
    PIN[1] = charKey[1];
    PIN[2] = charKey[2];
    PIN[3] = 0;

    // Debug info - Send the entered PIN to the serial device
    uBit.serial.send("PIN entered = ");
    uBit.serial.send(PIN);
    uBit.serial.send("\n");

    return;
}

/***********************************************************
 *
 * Function: onConnected()
 *
 * Description: Main program loop that is executed whilst
 *              BLE is in a connected state
 *
 **********************************************************/
void onConnected(MicroBitEvent e) {

    char PIN[4];
    char dpk[21];
    bool crcIsValid = false;
    int messageCrcValue, decodedCrcValue;

    uBit.display.scroll("C");
    uBit.serial.send("BLE Connected\n");

    connected = 1;

    // Main program loop.
    while (connected == 1) {

        // Wait until we have read the 32 byte ASCII message from the UART
        uart->read((uint8_t *) uartBuffer, 32, SYNC_SLEEP);

        // Terminate the string for display purposes
        uartBuffer[32] = '\0';

        // Debug printout for the received message
        uBit.serial.send("BLE Received message: ");
        uBit.serial.send(uartBuffer);
        uBit.serial.send("\n");

        // If the per session salt is not set yet, use
        // a hard coded PIN and Salt.
        if(perSessionSalt[0] == '\0') {
            char salt[] = "ThisIsMySaltThereAreManyLikeIt";
            strcpy(PIN, "123");

            generateDPK(dpk, salt, PIN);

        } else {
            // Per session salt already provided, used that with the
            // user entered PIN.
            // Read the PIN from the Microbit buttons
            readPIN(PIN);

            generateDPK(dpk, perSessionSalt, PIN);
        }

        // Decrypt the incoming message
        decryptMessage(dpk);

        // Validate that the incoming message has been decoded correctly
        GET_CRC(decodedAsciiMsg, messageCrcValue);

        messageCrcValue = (int)ASCII_TO_BCD(&decodedAsciiMsg[13]);

        decodedCrcValue = ccitt_crc(decodedAsciiMsg,13);

        if(messageCrcValue == decodedCrcValue) {
            crcIsValid = true;
        }

        if (IS_HEADER_VALID(decodedAsciiMsg) && crcIsValid ) {
            // Header and CRC are valid. Proceed with decoding the message
            int protocolVersion;
            int requestAcknowledge;
            int serviceID;
            int serviceData;

            uBit.serial.send("Decoded Message:- ");
            uBit.serial.send(decodedAsciiMsg);
            uBit.serial.send("\n");

            // Get the protocol version
            GET_PROTOCOL_VER(decodedAsciiMsg, protocolVersion);

            uBit.serial.send("Protocol Version:- ");
            uBit.serial.send(protocolVersion);
            uBit.serial.send("\n");

            // At the moment, there is only one protocol version
            // so backwards compatibility code is not required.
            if (protocolVersion == PROTOCOL_VERSION) {

                // Determine if this is a Request or Acknowledgement
                GET_REQ_ACK(decodedAsciiMsg, requestAcknowledge);

                uBit.serial.send("Request Acknowledge Flag:- ");
                uBit.serial.send(requestAcknowledge);
                uBit.serial.send("\n");

                if (requestAcknowledge == REQUEST) {
                    // Handle the service request

                    GET_SERVICE_ID(decodedAsciiMsg, serviceID);
                    GET_SERVICE_DATA(decodedAsciiMsg, serviceData);

                    switch (serviceID) {
                        case SERVICE_PER_SESSION_SALT:
                            // We are being sent a new per session salt to use
                            // to decode all future messages.
                            GET_SALT(decodedAsciiMsg, perSessionSalt);

                            uBit.serial.send("New Per Session Salt received. Salt = ");
                            uBit.serial.send(perSessionSalt);
                            uBit.serial.send("\n");
                            break;

                        case SERVICE_LED:
                            uBit.serial.send("LED Service Requested:- Request Data = ");
                            uBit.serial.send(serviceData);
                            uBit.serial.send("\n");

                            processLedRequest(serviceData);

                            break;

                        case SERVICE_BUZZER:
                            uBit.serial.send("Buzzer Service Requested:- Request Data = ");
                            uBit.serial.send(serviceData);
                            uBit.serial.send("\n");

                            processBuzzerRequest(serviceData);
                            break;

                        case SERVICE_FAN:
                            uBit.serial.send("Fan Service Requested:- Request Data = ");
                            uBit.serial.send(serviceData);
                            uBit.serial.send("\n");

                            processFanRequest(serviceData);
                            break;

                        case SERVICE_RGB_LED:
                            uBit.serial.send("RGB LED Service Requested:- Request Data = ");
                            uBit.serial.send(serviceData);
                            uBit.serial.send("\n");

                            processRgbLedRequest(serviceData);
                            break;

                        default:
                            // Currently Unsupported Service request.
                            // Send a notification back to the client
                            sendAck(SERVICE_ID_UNKNOWN, SERVICE_UNKNOWN, SERVICE_ACK_OK);
                            uBit.serial.send("Unsupported Service request:- ");
                            uBit.serial.send("\n");

                            break;
                    }
                } else {
                    // Currently no Acknowledgements are supported from the Android Client
                    uBit.serial.send("Unsupported Acknowledgement:- ");
                    uBit.serial.send("\n");
                }
            } else {
                // Unknown Protocol Version received. Currently only 1 version supported
                // No backwards compatibility code implemented at this time.
                uBit.serial.send("Unsupported Protocol Version:- ");
                uBit.serial.send("\n");
            }
        } else {
            // Message did not decode correctly and is invalid
            // Report incorrect pin used.
            sendAck(SERVICE_PIN, SERVICE_INCORRECT_PIN, SERVICE_ACK_FAILED);

            uBit.display.scrollAsync("Incorrect PIN");
        }
    } /* End while(connected) */
}

/***********************************************************
 *
 * Function: onDisconnected()
 *
 * Description: Handles the BLE disconnection event
 *
 **********************************************************/
void onDisconnected(MicroBitEvent e)
{
    uBit.display.scroll("D");
    uBit.serial.send ("BLE Disconnected\n");
    connected = 0;

    // Clear the per-session salt so that we can receive a new
    // one the next time the client connects
    perSessionSalt[0] = '\0';
}


/***********************************************************
 *
 * Function: onButtonA()
 *
 * Description: Handler for Button A event
 *
 **********************************************************/
void onButtonA(MicroBitEvent e)
{
    if (connected == 0) {
        uBit.display.scroll("NC");
        return;
    }else{
        if(!fButtonAWait){
            if(fCollectDigit0){
                digit[0]++;

                // Loop the digits if we go past 9
                if(digit[0] > 9) {
                    digit[0] = 0;
                }

                uBit.display.printChar(charlookup[digit[0]]);
            }else if(fCollectDigit1){
                digit[1]++;

                // Loop the digits if we go past 9
                if(digit[1] > 9) {
                    digit[1] = 0;
                }

                uBit.display.printChar(charlookup[digit[1]]);
            }else if(fCollectDigit2){
                digit[2]++;

                // Loop the digits if we go past 9
                if(digit[2] > 9) {
                    digit[2] = 0;
                }

                uBit.display.printChar(charlookup[digit[2]]);
            }
        } else {
            // Not currently waiting for a PIN
            uBit.display.printChar('A',100);
        }
    }
}

/***********************************************************
 *
 * Function: onButtonB()
 *
 * Description: Handler for Button B event
 *
 **********************************************************/
void onButtonB(MicroBitEvent e)
{
    if (connected == 0) {
        uBit.display.scroll("NC");
        return;
    }else{
        if(!fButtonBWait) {
            if(fCollectDigit0) {

                // disable both buttons and flash the entered digit 3 times
                fButtonAWait = true;
                fButtonBWait = true;

                // store the entered digit
                charKey[0] = charlookup[digit[0]];

                // begin displaying the next digit to be entered
                uBit.display.printChar(charlookup[digit[1]]);

                // swap our flags so the second digit will be worked on
                fCollectDigit0=false;
                fCollectDigit1=true;

                //re-enable both buttons
                fButtonAWait = false;
                fButtonBWait = false;

                // Debug output to send PIN digit 3 to serial device
                uBit.serial.printf("Digit 1 = %d\n",digit[0]);

            }else if(fCollectDigit1)
            {
                // disable both buttons and flash the entered digit 3 times
                fButtonAWait = true;
                fButtonBWait = true;

                // store the entered digit
                charKey[1] = charlookup[digit[1]];

                // begin displaying the next digit to be entered
                uBit.display.printChar(charlookup[digit[2]]);

                // swap our flags so the third digit will be worked on
                fCollectDigit1=false;
                fCollectDigit2=true;

                //re-enable both buttons
                fButtonAWait = false;
                fButtonBWait = false;

                // Debug output to send PIN digit 2 to serial device
                uBit.serial.printf("Digit 2 = %d\n",digit[1]);

            }else if(fCollectDigit2)
            {
                // disable both buttons and flash the entered digit 3 times
                fButtonAWait = true;
                fButtonBWait = true;

                // store the entered digit
                charKey[2] = charlookup[digit[2]];

                // clear the microbit display now that PIN is entered
                uBit.display.clear();

                // swap our flags so we know PIN entry is complete
                fCollectDigit2 = false;
                fPINCollected = true;

                //re-enable both buttons
                fButtonAWait = false;
                fButtonBWait = false;

                // Debug output to send PIN digit 3 to serial device
                uBit.serial.printf("Digit 3 = %d\n",digit[2]);

            }

        } else {
            // Not currently waiting for a PIN
            uBit.display.printChar('B',100);
        }
    }
}

/***********************************************************
 *
 * Function: onButtonAB()
 *
 * Description: Handler for both buttons event
 *
 **********************************************************/
void onButtonAB(MicroBitEvent e)
{
    if (connected == 0) {
        uBit.display.scroll("NC");
        return;
    }
    uBit.display.scroll("Both Buttons");
}


/***********************************************************
 *
 * Function: main()
 *
 * Description: Main program entry
 *
 **********************************************************/
int main()
{
    // Initialise the micro:bit runtime.
    uBit.init();

    // listen for Bluetooth connection state changes
    uBit.messageBus.listen(MICROBIT_ID_BLE, MICROBIT_BLE_EVT_CONNECTED, onConnected);
    uBit.messageBus.listen(MICROBIT_ID_BLE, MICROBIT_BLE_EVT_DISCONNECTED, onDisconnected);

    // listen for user button interactions
    uBit.messageBus.listen(MICROBIT_ID_BUTTON_A, MICROBIT_BUTTON_EVT_CLICK, onButtonA);
    uBit.messageBus.listen(MICROBIT_ID_BUTTON_B, MICROBIT_BUTTON_EVT_CLICK, onButtonB);
    uBit.messageBus.listen(MICROBIT_ID_BUTTON_AB, MICROBIT_BUTTON_EVT_CLICK, onButtonAB);


    // Note GATT table size increased from default in MicroBitConfig.h
    // #define MICROBIT_SD_GATT_TABLE_SIZE             0x500
    uart = new MicroBitUARTService(*uBit.ble, 32, 32);
    uBit.display.scrollAsync("IoT");
    uBit.sleep(200);
    uBit.serial.send("Microbit initialised\n");

    // If main exits, there may still be other fibers running or registered event handlers etc.
    // Simply release this fiber, which will mean we enter the scheduler. Worse case, we then
    // sit in the idle task forever, in a power efficient sleep.
    release_fiber();
}
