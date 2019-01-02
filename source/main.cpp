#include <stdbool.h>
#include "MicroBit.h"
#include "MicroBitUARTService.h"
#include "aes.h"
#include "sha1.h"
#include "main.h"
#include "protocol.h"
#include "utility.h"
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

bool LEDSoSOn = false;
bool LEDSoSFinished = true;

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
 * Function: decryptMessage()
 *
 * Description: Performs the decryption of the received
 *              message over BLE
 *
 **********************************************************/
void decryptMessage(char* dpk) {

    struct AES_ctx ctx;
    char hexBuffer[16];
    int i,j;

    // Convert the uart buffer message from ASCII to binary
    for (i = 0, j = 0; i < 32; j++, i += 2) {
        hexBuffer[j] = ASCII_TO_BCD(&(uartBuffer[i]));
    }

    // Decrypt the message. Output overwrites the input 'hexBuffer'
    AES_init_ctx(&ctx, (uint8_t*)dpk);
    AES_ECB_decrypt(&ctx, (uint8_t*)hexBuffer);

    // Copy the output message back to the global buffer 'decodedAsciiMsg'
    memcpy( &decodedAsciiMsg, &hexBuffer, 16 );

    // Terminate the string
    decodedAsciiMsg[15] = 0;
}
/***********************************************************
 *
 * Function: generateDPK()
 *
 * Description: Generates the DPK from the PIN + Salt.
 *              The combined string is passed through the SHA-1
 *              hash function. The result is then truncated to
 *              16 bytes (Size of the AES-128-ECB key) and used
 *              as the key.
 *
 **********************************************************/
void generateDPK(char dpk[21], char salt[], char PIN[4]) {

    // TODO - Change this size depending on our salt
    char dpkInput[34] = {0};
    char hexresult[41];
    size_t offset;

    // Combine the PIN and the salt into the DPK
    strcpy(dpkInput, PIN);
    strcat(dpkInput, salt);

    uBit.serial.send("DPK Input = ");
    uBit.serial.send(dpkInput);
    uBit.serial.send("\n");

    /* calculate hash on input string */
    SHA1(dpk, dpkInput, strlen(dpkInput));

    // Debug output for SHA1 Hash. Convert Hex to ASCII for printing. Trruncate to 16
    for (offset = 0; offset < 16; offset++) {
        sprintf((hexresult + (2 * offset)), "%02x", dpk[offset] & 0xff);
    }

    uBit.serial.send("Truncated SHA Output = ");
    uBit.serial.send(hexresult);
    uBit.serial.send("\n");

    // SHA1 produces a 20 byte output. AES-128-ECB only needs a 16 byte string,
    // Truncate the dpk.
    dpk[16] = 0;
}

/***********************************************************
 *
 * Function: sendAck()
 *
 * Description: Send the Acknowledgement back to the client.
 *              The protocol message is constructed based
 *              on the Service ID and status code and then sent
 *              over the BLE UART service.
 *
 **********************************************************/
void sendAck(int serviceID, int serviceCode, int status) {

    char buffer[32];

    // Set the IOT Header
    SET_HEADER(buffer);

    // Set the protocol version
    SET_PROTOCOL_VER(buffer, PROTOCOL_VERSION);

    // Set the Request/Acknowledgement flag
    SET_REQ_ACK(buffer, ACKNOWLEDGE);

    // Set the Service ID Flag;
    SET_SERVICE_ID(buffer, serviceID)

    // Set the status
    SET_SERVICE_DATA(buffer, status);

    // Set the Random Digit data
    SET_REDUNDANT_INFO(buffer);

    // Set the CRC in the message
    SET_CRC(buffer, ccitt_crc(buffer,13));

    buffer[15] = '\0';

    // Debug printout
    uBit.serial.send("Response Code: ");

    switch(serviceID) {
        case SERVICE_LED:
            uBit.serial.send("LED: ");

            switch (serviceCode) {
                case SERVICE_LED_OFF:
                    uBit.serial.send("Off ");
                    break;
                case SERVICE_LED_ON:
                    uBit.serial.send("On ");
                    break;
                case SERVICE_LED_SOS:
                    uBit.serial.send("SoS ");
                    break;
                default:
                    uBit.serial.send("Unknown ");
                    break;
            }

            break;
        case SERVICE_BUZZER:
            uBit.serial.send("BUZZER: ");
            switch (serviceCode) {
                case SERVICE_BUZZER_OFF:
                    uBit.serial.send("Off ");
                    break;
                case SERVICE_BUZZER_BASIC:
                    uBit.serial.send("Basic ");
                    break;
                case SERVICE_BUZZER_SIREN:
                    uBit.serial.send("Siren ");
                    break;
                case SERVICE_BUZZER_FANFARE:
                    uBit.serial.send("Fanfare ");
                    break;
                default:
                    uBit.serial.send("Unknown ");
                    break;

            }
            break;
        case SERVICE_RGB_LED:
            uBit.serial.send("RGB LED: ");
            switch (serviceCode) {
                case SERVICE_RGB_OFF:
                    uBit.serial.send("Off ");
                    break;
                case SERVICE_RGB_RED:
                    uBit.serial.send("Red ");
                    break;
                case SERVICE_RGB_BLUE:
                    uBit.serial.send("Blue ");
                    break;
                case SERVICE_RGB_GREEN:
                    uBit.serial.send("Green ");
                    break;
                case SERVICE_RGB_MAGENTA:
                    uBit.serial.send("Magenta ");
                    break;
                case SERVICE_RGB_YELLOW:
                    uBit.serial.send("Yellow ");
                    break;
                case SERVICE_RGB_CYAN:
                    uBit.serial.send("Cyan ");
                    break;
                case SERVICE_RGB_WHITE:
                    uBit.serial.send("White ");
                    break;
                case SERVICE_RGB_PARTY:
                    uBit.serial.send("Party ");
                    break;
                default:
                    uBit.serial.send("Unknown ");
                    break;

            }
            break;
        case SERVICE_FAN:
            uBit.serial.send("FAN: ");
            switch (serviceCode) {

                case SERVICE_FAN_OFF:
                    uBit.serial.send("Off ");
                    break;
                case SERVICE_FAN_SLOW:
                    uBit.serial.send("Slow ");
                    break;
                case SERVICE_FAN_MED:
                    uBit.serial.send("Medium ");
                    break;
                case SERVICE_FAN_FAST:
                    uBit.serial.send("Fast ");
                    break;
                default:
                    uBit.serial.send("Unknown ");
                    break;

            }
            break;

        case SERVICE_PIN:
            switch(serviceCode) {
                case SERVICE_INCORRECT_PIN:
                    uBit.serial.send("Incorrect PIN ");
                    break;
                default:
                    break;
            }
            break;

        case SERVICE_ID_UNKNOWN:
            switch(serviceCode) {
                default:
                    uBit.serial.send("UNKNOWN ");
                    break;
            }
            break;

        default:
            uBit.serial.send("UNKNOWN SERVICE");
            break;
    }

    switch(status) {
        case SERVICE_ACK_OK:
            uBit.serial.send("- OK ");
            break;
        case SERVICE_ACK_FAILED:
            uBit.serial.send("- FAILED! ");
            break;
        default:
            break;
    }

    uBit.serial.send("\n");

    uart->send(buffer);

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
    int messageCrcValue;

    uBit.display.scroll("C");
    uBit.serial.send("BLE Connected\n");

    connected = 1;

    // Main program loop.
    while (connected == 1) {

        // Wait until we have read the 32 byte ASCII message from the UART
        uart->read((uint8_t *) uartBuffer, 32, SYNC_SLEEP);

        // Terminate the string correctly
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
            // Per sessoion salt already provided, used that with the
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

        char tmpBuf[6];
        sprintf(tmpBuf, "%01x\n", messageCrcValue);
        uBit.serial.send("Message CRC Value: ");
        uBit.serial.send(tmpBuf);

        // Sleep to allow the serial send to finish before doing
        // another. This prevents a uBit crash.
        uBit.sleep(200);

        int checksum = ccitt_crc(decodedAsciiMsg,13);

        sprintf(tmpBuf, "%01x\n", checksum);
        uBit.serial.send("Calculated CRC Value: ");
        uBit.serial.send(tmpBuf);

        if(messageCrcValue == checksum) {
            crcIsValid = true;
        }

        // TODO - Remove when Gwyn has the android code
        crcIsValid = true;

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
                        case SERVICE_PER_SERSSION_SALT:
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

                            if (serviceData == SERVICE_LED_ON) {

                                // If the SoS function is running, wait for it to
                                // finish its round.
                                if(LEDSoSOn) {
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

                                // TODO - Cannot use while(1)
//                                while(1){
                                create_fiber(LED_SOS);
//                                }

                                // Send a notification back to the client
                                sendAck(SERVICE_LED, SERVICE_LED_SOS, SERVICE_ACK_OK);

                                // Display the result on the LEDs
                                uBit.display.scrollAsync("LED SOS");

                            } else {
                                // Send a notification back to the client
                                sendAck(SERVICE_LED, SERVICE_UNKNOWN, SERVICE_ACK_FAILED);
                            }
                            break;

                        case SERVICE_BUZZER:
                            uBit.serial.send("Buzzer Service Requested:- Request Data = ");
                            uBit.serial.send(serviceData);
                            uBit.serial.send("\n");

                            if (serviceData == SERVICE_BUZZER_BASIC) {
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

                                // Send a notification back to the client
                                sendAck(SERVICE_BUZZER, SERVICE_BUZZER_OFF, SERVICE_ACK_OK);

                                // Display the result on the LEDs
                                uBit.display.scrollAsync("Buzzer off");

                            } else if (serviceData == SERVICE_BUZZER_SIREN) {
                                // Make buzzer act as a siren
                                // TODO - Cannot use while(1)
//                                while(1){
                                buzzSiren();
//                                }
                                // Send a notification back to the client
                                sendAck(SERVICE_BUZZER, SERVICE_BUZZER_SIREN, SERVICE_ACK_OK);

                                // Display the result on the LEDs
                                uBit.display.scrollAsync("Buzzer Siren");

                            } else if (serviceData == SERVICE_BUZZER_FANFARE) {
                                // Make buzzer play a fanfare
                                // TODO - Cannot use while(1)
//                                while(1){
                                buzzFanfare();
//                                  }
                                // Send a notification back to the client
                                sendAck(SERVICE_BUZZER, SERVICE_BUZZER_FANFARE, SERVICE_ACK_OK);

                                // Display the result on the LEDs
                                uBit.display.scrollAsync("Buzzer Fanfare");

                            } else {
                                // Send a notification back to the client
                                sendAck(SERVICE_BUZZER, SERVICE_UNKNOWN, SERVICE_ACK_FAILED);
                            }
                            break;

                        case SERVICE_FAN:
                            uBit.serial.send("Fan Service Requested:- Request Data = ");
                            uBit.serial.send(serviceData);
                            uBit.serial.send("\n");

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
                            break;

                        case SERVICE_RGB_LED:
                            uBit.serial.send("RGB LED Service Requested:- Request Data = ");
                            uBit.serial.send(serviceData);
                            uBit.serial.send("\n");

                            if (serviceData == SERVICE_RGB_OFF) {
                                // Switch off the power to the Fan
                                FAN.setDigitalValue(0);

                                // Send a notification back to the client
                                sendAck(SERVICE_RGB_LED, SERVICE_RGB_OFF, SERVICE_ACK_OK);

                                // Display the result on the LEDs
                                uBit.display.scrollAsync("RGB LED off");

                            } else if (serviceData == SERVICE_RGB_RED) {
                                // Set RGB LED to Red
                                RGB_RED.setDigitalValue(1);
                                RGB_GREEN.setDigitalValue(0);
                                RGB_BLUE.setDigitalValue(0);

                                // Send a notification back to the client
                                sendAck(SERVICE_RGB_LED, SERVICE_RGB_RED, SERVICE_ACK_OK);

                                // Display the result on the LEDs
                                uBit.display.scrollAsync("RGB LED setting: Red");

                            } else if (serviceData == SERVICE_RGB_BLUE) {
                                // Set RGB LED to Blue
                                RGB_RED.setDigitalValue(0);
                                RGB_GREEN.setDigitalValue(0);
                                RGB_BLUE.setDigitalValue(1);

                                // Send a notification back to the client
                                sendAck(SERVICE_RGB_LED, SERVICE_RGB_BLUE, SERVICE_ACK_OK);

                                // Display the result on the LEDs
                                uBit.display.scrollAsync("RGB LED setting: Blue");

                            } else if (serviceData == SERVICE_RGB_GREEN) {
                                // Set RGB LED to Green
                                RGB_RED.setDigitalValue(0);
                                RGB_GREEN.setDigitalValue(1);
                                RGB_BLUE.setDigitalValue(0);

                                // Send a notification back to the client
                                sendAck(SERVICE_RGB_LED, SERVICE_RGB_GREEN, SERVICE_ACK_OK);

                                // Display the result on the LEDs
                                uBit.display.scrollAsync("RGB LED setting: Green");

                            } else if (serviceData == SERVICE_RGB_MAGENTA) {
                                // Set RGB LED to Magenta
                                RGB_RED.setDigitalValue(1);
                                RGB_GREEN.setDigitalValue(0);
                                RGB_BLUE.setDigitalValue(1);

                                // Send a notification back to the client
                                sendAck(SERVICE_RGB_LED, SERVICE_RGB_MAGENTA, SERVICE_ACK_OK);

                                // Display the result on the LEDs
                                uBit.display.scrollAsync("RGB LED setting: Magenta");

                            } else if (serviceData == SERVICE_RGB_YELLOW) {
                                // Set RGB LED to Yellow
                                RGB_RED.setDigitalValue(1);
                                RGB_GREEN.setDigitalValue(1);
                                RGB_BLUE.setDigitalValue(0);

                                // Send a notification back to the client
                                sendAck(SERVICE_RGB_LED, SERVICE_RGB_YELLOW, SERVICE_ACK_OK);

                                // Display the result on the LEDs
                                uBit.display.scrollAsync("RGB LED setting: Yellow");

                            } else if (serviceData == SERVICE_RGB_CYAN) {
                                // Set RGB LED to Cyan
                                RGB_RED.setDigitalValue(0);
                                RGB_GREEN.setDigitalValue(1);
                                RGB_BLUE.setDigitalValue(1);

                                // Send a notification back to the client
                                sendAck(SERVICE_RGB_LED, SERVICE_RGB_CYAN, SERVICE_ACK_OK);

                                // Display the result on the LEDs
                                uBit.display.scrollAsync("RGB LED setting: Cyan");

                            } else if (serviceData == SERVICE_RGB_WHITE) {
                                // Set RGB LED to White
                                RGB_RED.setDigitalValue(1);
                                RGB_GREEN.setDigitalValue(1);
                                RGB_BLUE.setDigitalValue(1);

                                // Send a notification back to the client
                                sendAck(SERVICE_RGB_LED, SERVICE_RGB_WHITE, SERVICE_ACK_OK);

                                // Display the result on the LEDs
                                uBit.display.scrollAsync("RGB LED setting: White");

                            } else if (serviceData == SERVICE_RGB_PARTY) {
                                // Activate Party Mode - Merry Christmas!
                                // TODO - Cannot use while(1)
//                                while(1){
                                rgbParty();
//                              }

                                // Send a notification back to the client
                                sendAck(SERVICE_RGB_LED, SERVICE_RGB_PARTY, SERVICE_ACK_OK);

                                // Display the result on the LEDs
                                uBit.display.scrollAsync("LET'S PARTY");

                            } else {
                                // Unsupported RGB LED action.
                                // Send a notification back to the client
                                sendAck(SERVICE_RGB_LED, SERVICE_UNKNOWN, SERVICE_ACK_FAILED);
                            }
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
                // No backwards compatibility code implemented at the moment.
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
//                flashScreen();

//                uBit.display.clear();
//                uBit.sleep(200);

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
                //TODO - Remove

                // disable both buttons and flash the entered digit 3 times
                fButtonAWait = true;
                fButtonBWait = true;
//                flashScreen();

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
//                flashScreen();

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
