#include "MicroBit.h"
#include "MicroBitUARTService.h"
#include "aes.h"
#include "sha1.h"
#include "main.h"
#include "communication.h"
#include "utility.h"
#include "protocol.h"
#include "ccitt-crc.h"

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

    char messageBuffer[32];

    // Set the IOT Header
    SET_HEADER(messageBuffer);

    // Set the protocol version
    SET_PROTOCOL_VER(messageBuffer, PROTOCOL_VERSION);

    // Set the Request/Acknowledgement flag
    SET_REQ_ACK(messageBuffer, ACKNOWLEDGE);

    // Set the Service ID Flag;
    SET_SERVICE_ID(messageBuffer, serviceID)

    // Set the status
    SET_SERVICE_DATA(messageBuffer, status);

    // Set the Random Digit data
    SET_REDUNDANT_INFO(messageBuffer);

    // Set the CRC in the message
    SET_CRC(messageBuffer, ccitt_crc(messageBuffer,13));

    messageBuffer[15] = '\0';

    // Debug printout
    uBit.serial.send("Response Code: " );

    uBit.sleep(100);

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

    uart->send(messageBuffer);

}
