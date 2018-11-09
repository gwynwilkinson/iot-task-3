#include "MicroBit.h"
#include "MicroBitUARTService.h"
#include "aes.h"

MicroBit uBit;
MicroBitUARTService *uart;
MicroBitPin P1(MICROBIT_ID_IO_P1, MICROBIT_PIN_P1, PIN_CAPABILITY_DIGITAL);

int connected = 0;

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

    connected = 1;

    // The Android client will send ASCII strings terminated with the colon character
    ManagedString eom(":");

    // Main program loop.
    while (connected == 1) {

        // Wait until we have read the message from the UART
        ManagedString msg = uart->readUntil(eom);

        // TO-DO - Read the ping from the Microbit buttons

        // TO-DO - Combine the pin and the salt into the DPK

        // TO-DO - Decrypt the incoming message

        // TO-DO - Validate the decrypted message is valid

        // TO-DO - Tidy up the below code into another function to handle the protocol message

        // Check the received PIN and control the relevant device.
        if (msg == "999")
        {
            P1.setDigitalValue(1);
            uart->send("LED On");
            uBit.display.scrollAsync("LED on");

        } else if ( msg == "998") {
            P1.setDigitalValue(0);
            uart->send("LED Off");
            uBit.display.scrollAsync("LED off");
        } else {
            char sendBuffer[50];

            sprintf(sendBuffer, "Unknown : %s", msg.toCharArray());

            uBit.serial.send(sendBuffer);

            uart->send(sendBuffer);

            uBit.display.scrollAsync("???: ");
            uBit.display.scrollAsync(msg);
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
    uBit.display.scroll("Button A");
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

// TO-DO - Remove this code once we have the main loop decryption in. Reference code for Jon at the moment.
static int test_decrypt_ecb(void)
{

    uint8_t key[] = { 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    uint8_t in[]  = { 0x09, 0x03, 0x85, 0xa5, 0x66, 0xf9, 0x7f, 0x47, 0x93, 0xde, 0x85, 0xd7, 0x0b, 0x36, 0x86, 0x4d };

    struct AES_ctx ctx;
    char serialBuffer[18];


    AES_init_ctx(&ctx, key);
    AES_ECB_decrypt(&ctx, in);

    for( int offset = 0; offset < 16; offset++) {
        sprintf( ( serialBuffer + (2*offset)), "%02x", in[offset]&0xff);
    }

    sprintf(serialBuffer + 16 , "\n");
    uBit.serial.send("Decoded String = ");
    uBit.serial.send((char*)in);
    uBit.display.scrollAsync(serialBuffer);

    uBit.serial.send("\n");
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
    uBit.display.scrollAsync("IoT BLE test");

    // Temp code to test the decrypt routines
    test_decrypt_ecb();

    // If main exits, there may still be other fibers running or registered event handlers etc.
    // Simply release this fiber, which will mean we enter the scheduler. Worse case, we then
    // sit in the idle task forever, in a power efficient sleep.
    release_fiber();
}


