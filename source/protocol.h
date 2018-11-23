#ifndef IOT_TASK_3_PROTOCOL_H
#define IOT_TASK_3_PROTOCOL_H

// Defines for protocol elements
# define PROTOCOL_VERSION 1

# define REQUEST 0
# define ACKNOWLEDGE 1

// Defines for service IDs and Data

#define SERVICE_LED 1
#define SERVICE_BUZZER 2
#define SERVICE_RGB_LED 3
#define SERVICE_FAN 4

// Service Data for LED
#define SERVICE_LED_OFF 0
#define SERVICE_LED_ON 1
#define SERVICE_LED_SOS 2

// Service Data for Buzzer
#define SERVICE_BUZZER_OFF 0
#define SERVICE_BUZZER_BASIC 1
#define SERVICE_BUZZER_SIREN 2
#define SERVICE_BUZZER_FANFARE 3

// Service Data for RGB Led
#define SERVICE_RGB_OFF 0
#define SERVICE_RGB_RED 1
#define SERVICE_RGB_BLUE 2
#define SERVICE_RGB_GREEN 3
#define SERVICE_RGB_MAGENTA 4
#define SERVICE_RGB_YELLOW 5
#define SERVICE_RGB_CYAN 6
#define SERVICE_RGB_WHITE 7
#define SERVICE_RGB_PARTY 8

// Service Data for Fan
#define SERVICE_FAN_OFF 0
#define SERVICE_FAN_SLOW 1
#define SERVICE_FAN_MED 2
#define SERVICE_FAN_FAST 3

// Protocol Message definition

//  -------------------------------------------------------------------------------------------
// | Byte        | 0 | 1 | 2 |  3  |   4  | 5   | 6 | 7 | 8 |  9 | 10 | 11 | 12 | 13 | 14 | 15 |
//  -------------------------------------------------------------------------------------------
// | Description |   Header  | Prot | Req | Svc |    Service Data          |  Redund |   CRC   |
// |             |           | Ver  | ACK | ID  |                          |   Info  |         |
//  -------------------------------------------------------------------------------------------
// | Contents    | I   O   T |   1  | 0/1 | 0-F |   0 - FFFFFF             |  Random |  0-FF   |
//  -------------------------------------------------------------------------------------------

// TODO - Remove this example code
//
// Gwyn - This is how you can call the SET and GET for these macros.
// The GET's  generally return an INT, except for the service data which is a 'long long'
//
//SET_HEADER(decodedAsciiMsg);
//SET_PROTOCOL_VER(decodedAsciiMsg,PROTOCOL_VERSION);
//SET_REQ_ACK(decodedAsciiMsg,ACKNOWLEDGE);
//SET_SERVICE_ID(decodedAsciiMsg,4);
//SET_SERVICE_DATA(decodedAsciiMsg,123456);
//SET_REDUNDANT_INFO(decodedAsciiMsg,1234);
//SET_CRC(decodedAsciiMsg,9876);
//
//GET_PROTOCOL_VER(decodedAsciiMsg, ver);
//GET_REQ_ACK(decodedAsciiMsg, ack);
//GET_SERVICE_ID(decodedAsciiMsg, sID);
//GET_SERVICE_DATA(decodedAsciiMsg, sData);
//GET_CRC(decodedAsciiMsg, crc);
// TODO - End example code

// Set the header of the string to 'IoT'
#define SET_HEADER(stringArray) { \
    stringArray[0] = 'I'; \
    stringArray[1] = 'o'; \
    stringArray[2] = 'T'; \
}

// Returns 1 if the header matches
#define IS_HEADER_VALID(array) (strncmp(array, "IoT", 3) ? 0 : 1)

#define SET_PROTOCOL_VER(array, version) { \
    array[3] = (char)version; \
}

#define GET_PROTOCOL_VER(array, version) { \
    version = (int)array[3] - 0x30; \
}

#define SET_REQ_ACK(array, value) { \
    array[4] = (char)value; \
}

#define GET_REQ_ACK(array, value) { \
    value = (int)array[4] - 0x30; \
}

#define SET_SERVICE_ID(array, serviceID) { \
    array[5] = (char)serviceID; \
}

#define GET_SERVICE_ID(array, serviceID) { \
    serviceID = (int)array[5] - 0x30; \
}

#define SET_SERVICE_DATA(array, serviceData) { \
    array[6] = ((serviceData & 0xFF00000000) >> 32); \
    array[7] = ((serviceData & 0xFF000000) >> 24); \
    array[8] = ((serviceData & 0xFF0000) >> 16); \
    array[9] = ((serviceData & 0xFF00) >> 8); \
    array[10] = ((serviceData & 0xFF) ); \
}

#define GET_SERVICE_DATA(array, serviceData) { \
    serviceData = (int)(((array[6]) - 0x30)<< 24); \
    serviceData |= (int)(((array[7]) - 0x30) << 24); \
    serviceData |= (int)(((array[8]) - 0x30) << 16); \
    serviceData |= (int)(((array[9]) -0x30)<< 8); \
    serviceData |= (int)array[10] - 0x30; \
}

#define SET_REDUNDANT_INFO(array, randomNumber) { \
    array[11] = ((randomNumber & 0xFF00) >> 8); \
    array[12] = (randomNumber & 0xFF); \
}

#define SET_CRC(array, crcValue) { \
    array[13] = ((crcValue & 0xFF00) >> 8); \
    array[14] = (crcValue & 0xFF); \
}

#define GET_CRC(array, crcValue) { \
    crcValue = (array[13] << 8) - 0x30; \
    crcValue |= array[14] - 0x30; \
}

#endif //IOT_TASK_3_PROTOCOL_H