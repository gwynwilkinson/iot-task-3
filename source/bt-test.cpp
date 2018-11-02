#include "MicroBit.h"
#include "MicroBitUARTService.h"
#include "aes.h"

MicroBit uBit;
MicroBitUARTService *uart;
MicroBitPin P1(MICROBIT_ID_IO_P1, MICROBIT_PIN_P1, PIN_CAPABILITY_DIGITAL);

int connected = 0;

void onConnected(MicroBitEvent e)
{
    uBit.display.scroll("C");

    connected = 1;

    // my client will send ASCII strings terminated with the colon character
    ManagedString eom(":");

    while (connected == 1) {
        ManagedString msg = uart->readUntil(eom);

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

void onDisconnected(MicroBitEvent e)
{
    uBit.display.scroll("D");
    connected = 0;
}

void onButtonA(MicroBitEvent e)
{
    if (connected == 0) {
        uBit.display.scroll("NC");
        return;
    }
    uart->send("Button A");
    uBit.display.scroll("Button A");
}

void onButtonB(MicroBitEvent e)
{
    if (connected == 0) {
        uBit.display.scroll("NC");
        return;
    }
    uart->send("Button B");
    uBit.display.scroll("Button B");
}

void onButtonAB(MicroBitEvent e)
{
    if (connected == 0) {
        uBit.display.scroll("NC");
        return;
    }
    uart->send("Both Buttons");
    uBit.display.scroll("Both Buttons");
}


static void test_encrypt_ecb_verbose(void)
{
    // Example of more verbose verification

    uint8_t i;
    char serialBuffer[50];

    // 128bit key
    uint8_t key[16] =        { (uint8_t) 0x30, (uint8_t) 0x00, (uint8_t) 0x00, (uint8_t) 0x00, (uint8_t) 0x00, (uint8_t) 0x00, (uint8_t) 0x00, (uint8_t) 0x00,
                               (uint8_t) 0x00, (uint8_t) 0x00, (uint8_t) 0x00, (uint8_t) 0x00, (uint8_t) 0x00, (uint8_t) 0x00, (uint8_t) 0x00, (uint8_t) 0x00 };
    // 512bit text
    uint8_t plain_text[16] = { (uint8_t) 0x55, (uint8_t) 0x57, (uint8_t) 0x45, (uint8_t) 0x00, (uint8_t) 0x00, (uint8_t) 0x00, (uint8_t) 0x00, (uint8_t) 0x00,
                               (uint8_t) 0x00, (uint8_t) 0x00, (uint8_t) 0x00, (uint8_t) 0x00, (uint8_t) 0x00, (uint8_t) 0x00, (uint8_t) 0x00, (uint8_t) 0x00 };


    struct AES_ctx ctx;
    AES_init_ctx(&ctx, key);


    AES_ECB_encrypt(&ctx, plain_text );
    for( int offset = 0; offset < 16; offset++) {
        sprintf( ( serialBuffer + (2*offset)), "%02x", plain_text[offset]&0xff);
    }

    sprintf(serialBuffer + 16 , "\n");
    uBit.serial.send("AES-128 Cipher = ");
    uBit.serial.send(serialBuffer);
    uBit.display.scrollAsync((char*)serialBuffer);

}

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
    uBit.display.scrollAsync((char*)in);

    uBit.serial.send("\n");
}

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

    test_encrypt_ecb_verbose();
    test_decrypt_ecb();

    // If main exits, there may still be other fibers running or registered event handlers etc.
    // Simply release this fiber, which will mean we enter the scheduler. Worse case, we then
    // sit in the idle task forever, in a power efficient sleep.
    release_fiber();
}


