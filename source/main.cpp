#include "MicroBit.h"
#include "MicroBitUARTService.h"
#include "aes.h"
#include "utility.h"

MicroBit uBit;
MicroBitUARTService *uart;
MicroBitPin P1(MICROBIT_ID_IO_P1, MICROBIT_PIN_P1, PIN_CAPABILITY_DIGITAL);

int connected = 0;

char uartBuffer[33];
char decodedAsciiMsg[33];

/***********************************************************
 *
 * Function: decryptMessage()
 *
 * Description: Performs the decryption of the received
 *              message over BLE
 *
 **********************************************************/
void decryptMessage() {

    struct AES_ctx ctx;
    char hexBuffer[16];
    char serialBuffer[33];
    int i,j;

    // TODO - remove the hard coded key and read from the microbit buttons
    uint8_t key[] = { 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

    // Convert the uart buffer message from ASCII to binary
    for (i = 0, j = 0; i < 32; j++, i += 2) {
        hexBuffer[j] = ASCII_TO_BCD(&(uartBuffer[i]));
    }

    // Decrypt the message. Output overwrites the input 'hexBuffer'
    AES_init_ctx(&ctx, key);
    AES_ECB_decrypt(&ctx, (uint8_t*)hexBuffer);

    // Copy the output message back to the global buffer 'decodedAsciiMsg'
    memcpy( &decodedAsciiMsg, &hexBuffer, 32 );

    // Terminate the string
    // TODO - Change this to 32 when we are using all the buffer instead of 3 digit PIN
    decodedAsciiMsg[4] = 0;

    uBit.serial.send("Decoded String = ");
    uBit.serial.send(decodedAsciiMsg);


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
    uBit.display.scroll("C");
    uBit.serial.send ("BLE Connected\n");

    connected = 1;

    // The Android client will send ASCII strings terminated with the colon character
    ManagedString eom(":");

    // Main program loop.
    while (connected == 1) {

        // Wait until we have read the message from the UART
        int charsRead = uart->read((uint8_t*)uartBuffer, 32, SYNC_SLEEP );

        // Terminate the string correctly
        uartBuffer[32] = '\0';

        // Debug printout for the received message
        uBit.serial.send("BLE Received message: ");
        uBit.serial.send(uartBuffer);
        uBit.serial.send("\n");

        // TO-DO - Read the PIN from the Microbit buttons

        // TO-DO - Combine the PIN and the salt into the DPK

        // TO-DO - Decrypt the incoming message
        decryptMessage();

        // TO-DO - Validate the decrypted message is valid

        // TO-DO - Tidy up the below code into another function to handle the protocol message
        // Check the received PIN and control the relevant device.
        if (strcmp(decodedAsciiMsg,"999"))
        {
            P1.setDigitalValue(1);
            uart->send("LED On");
            uBit.display.scrollAsync("LED on");

        } else if ( strcmp(decodedAsciiMsg,"998")) {
            P1.setDigitalValue(0);
            uart->send("LED Off");
            uBit.display.scrollAsync("LED off");
        } else {

            // Output debug information regarding the unknown PIN
            char sendBuffer[50];
            sprintf(sendBuffer, "Unknown PIN: %s", decodedAsciiMsg);
            uBit.serial.send(sendBuffer);
            uart->send(sendBuffer);

            // Display the unknown PIN on the Microbit LEDs
//            uBit.display.scrollAsync("?: ");
//            uBit.display.scrollAsync(decodeAsciiMsg);
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
    }
    uBit.display.scroll(uart->rxBufferedSize());
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
    }
    uBit.display.scroll("Button B");
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
    uBit.serial.send("Microbit initialised\n");

    // If main exits, there may still be other fibers running or registered event handlers etc.
    // Simply release this fiber, which will mean we enter the scheduler. Worse case, we then
    // sit in the idle task forever, in a power efficient sleep.
    release_fiber();
}


