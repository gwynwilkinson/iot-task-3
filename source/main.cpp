#include <stdbool.h>
#include "MicroBit.h"
#include "MicroBitUARTService.h"
#include "aes.h"
#include "sha1.h"
#include "protocol.h"
#include "utility.h"
#include <string.h>


MicroBit uBit;
MicroBitUARTService *uart;
MicroBitPin P1(MICROBIT_ID_IO_P1, MICROBIT_PIN_P1, PIN_CAPABILITY_DIGITAL);

int connected = 0;

char uartBuffer[33];
char decodedAsciiMsg[33];

static uint8_t hexlookup[] = {0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39};
static char charlookup[] = "0123456789";

uint8_t hexKey[16] = {0x00, 0x00, 0x00, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B};
unsigned char charKey[] = "000";

bool fCollectDigit0;
bool fCollectDigit1;
bool fCollectDigit2;

bool fPINCollected;

unsigned int digit[3] = {0,0,0};

bool fButtonAWait = false;
bool fButtonBWait = false;


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
        uBit.sleep(200);
    }

    // translates entered digits into decimal integer and returns to caller
    PIN[0] = charKey[0];
    PIN[1] = charKey[1];
    PIN[2] = charKey[2];
    PIN[3] = 0;

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
    // TODO - Change this to 16 when we are using all the buffer instead of IoT + 3 digit PIN
    decodedAsciiMsg[7] = 0;
}

/***********************************************************
 *
 * Function: onConnected()
 *
 * Description: Main program loop that is executed whilst
 *              BLE is in a connected state
 *
 **********************************************************/
void onConnected(MicroBitEvent e)
{
    char PIN[4] = "999";
    char salt[] = "ThisIsMySaltThereAreManyLikeIt";

    uBit.display.scroll("C");
    uBit.serial.send ("BLE Connected\n");

    // TODO - Change this size depending on our salt
    char dpkInput[34] = {0};
    char dpk[21];

    char hexresult[41];
    size_t offset;

    connected = 1;

    // Main program loop.
    while (connected == 1) {

        // Wait until we have read the 32 byte ASCII message from the UART
        uart->read((uint8_t*)uartBuffer, 32, SYNC_SLEEP );

        // Terminate the string correctly
        uartBuffer[32] = '\0';

        // Debug printout for the received message
        uBit.serial.send("BLE Received message: ");
        uBit.serial.send(uartBuffer);
        uBit.serial.send("\n");

        // Read the PIN from the Microbit buttons
        readPIN(PIN);

        uBit.serial.send("PIN entered = ");
        uBit.serial.send(PIN);
        uBit.serial.send("\n");

        // Combine the PIN and the salt into the DPK
        strcpy(dpkInput, PIN);
        strcat(dpkInput,salt);

        uBit.serial.send("DPK Input = ");
        uBit.serial.send(dpkInput);
        uBit.serial.send("\n");

        /* calculate hash on input string */
        SHA1( dpk, dpkInput, strlen(dpkInput) );

        // Debug output for SHA1 Hash. Convert Hex to ASCII for printing. Trruncate to 16
        for( offset = 0; offset < 16; offset++) {
            sprintf( ( hexresult + (2*offset)), "%02x", dpk[offset]&0xff);
        }

        uBit.serial.send("Truncated SHA Output = ");
        uBit.serial.send(hexresult);
        uBit.serial.send("\n");

        // SHA1 produces a 20 byte output. AES-128-ECB only needs a 16 byte string,
        // Truncate the dpk.
        dpk[16] = 0;

        // Decrypt the incoming message
        decryptMessage(dpk);

        // TODO - Validate the decrypted message is valid
        if (IS_HEADER_VALID(decodedAsciiMsg)) {
            uBit.serial.send("Decoded Message:- ");
            uBit.serial.send(decodedAsciiMsg);
            uBit.serial.send("\n");
        }

        // TODO - Tidy up the below code into another function to handle the protocol message
        // Check the received PIN and control the relevant device.
        if (0 == strcmp(decodedAsciiMsg,"IoT-100"))
        {
            // Switch on the power to the LED
            P1.setDigitalValue(1);

            // Send a notification back to the phone
            uart->send("LED On");

            // Display the result on the LEDs
            uBit.display.scrollAsync("LED on");

            // Send the debug info to the serial device
            uBit.serial.send("PIN 100 - LED on");
            uBit.serial.send("\n");

        } else if (0 == strcmp(decodedAsciiMsg,"IoT-101")) {

            // Switch off the power to the LED
            P1.setDigitalValue(0);

            // Send a notification back to the phone
            uart->send("LED Off");

            // Display the result on the LEDs
            uBit.display.scrollAsync("LED off");

            // Send the debug info to the serial device
            uBit.serial.send("PIN 101 - LED off");
            uBit.serial.send("\n");

        } else if (0 == strncmp(decodedAsciiMsg, "IoT", 3)) {

            // Display the result on the LEDs
            uBit.display.scrollAsync("???");

//            // Send the debug info to the serial device
//            uBit.serial.send("Unsupported PIN:- ");
//            uBit.serial.send(decodedAsciiMsg);
//            uBit.serial.send("\n");

        } else {

//            // Send a notification back to the phone
//            uart->send("Incorrect PIN");

            // Display the result on the LEDs
            uBit.display.scrollAsync("Incorrect PIN");

//            // Output debug information regarding the unknown PIN
//            uBit.serial.send("Incorrect PIN\n");

        }
    }

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
}
/***********************************************************
 *
 * Function: flashScreen()
 *
 * Description: Flashes the Microbit display three times
 *
 **********************************************************/
void flashScreen(){
    uBit.display.disable();
    uBit.sleep(200);
    uBit.display.enable();
    uBit.sleep(200);
    uBit.display.disable();
    uBit.sleep(200);
    uBit.display.enable();
    uBit.sleep(200);
    uBit.display.disable();
    uBit.sleep(200);
    uBit.display.enable();
    uBit.sleep(200);
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
                if(digit[2] > 9) {
                    digit[2] = 0;
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
                flashScreen();

                // store the entered digit in all sorts of formats
                hexKey[0] = hexlookup[digit[0]];
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
                uBit.serial.printf("Digit 1 = %d",digit[0]);
                uBit.serial.send("\n");

            }else if(fCollectDigit1)
            {
                // disable both buttons and flash the entered digit 3 times
                fButtonAWait = true;
                fButtonBWait = true;
                flashScreen();

                // store the entered digit in all sorts of formats
                hexKey[1] = hexlookup[digit[1]];
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
                uBit.serial.printf("Digit 2 = %d",digit[1]);
                uBit.serial.send("\n");

            }else if(fCollectDigit2)
            {
                // disable both buttons and flash the entered digit 3 times
                fButtonAWait = true;
                fButtonBWait = true;
                flashScreen();

                // store the entered digit in all sorts of formats
                hexKey[2] = hexlookup[digit[2]];
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
                uBit.serial.printf("Digit 3 = %d",digit[2]);
                uBit.serial.send("\n");

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
    uBit.display.scrollAsync("IoT BLE");
    uBit.sleep(200);
    uBit.serial.send("Microbit initialised\n");
//    if(connected != 0){
//        int PIN1 = readPIN();
//        uBit.sleep(200);
//        ManagedString mStr(PIN1);
//        uBit.display.scroll(mStr);
//        uBit.serial.send(mStr);
//    }

    // If main exits, there may still be other fibers running or registered event handlers etc.
    // Simply release this fiber, which will mean we enter the scheduler. Worse case, we then
    // sit in the idle task forever, in a power efficient sleep.
    release_fiber();
}
