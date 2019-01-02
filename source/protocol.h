#ifndef IOT_TASK_3_PROTOCOL_H
#define IOT_TASK_3_PROTOCOL_H

// Defines for protocol elements
# define PROTOCOL_VERSION 1

# define REQUEST 0
# define ACKNOWLEDGE 1

// Defines for service IDs and Data
#define SERVICE_PER_SERSSION_SALT 0
#define SERVICE_LED 1
#define SERVICE_BUZZER 2
#define SERVICE_RGB_LED 3
#define SERVICE_FAN 4
#define SERVICE_PIN 8
#define SERVICE_ID_UNKNOWN 9

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

// Service Data for Acknowledgements
#define SERVICE_ACK_FAILED 0
#define SERVICE_ACK_OK 1

#define SERVICE_INCORRECT_PIN 9
#define SERVICE_UNKNOWN 99

// Protocol Message definition

//  ---------------------------------------------------------------------------------------
// | Byte        | 0 | 1 | 2 |  3  |   4  | 5   | 6 | 7 | 8 |  9 | 10 | 11 | 12 | 13 | 14 |
//  ---------------------------------------------------------------------------------------
// | Description |   Header  | Prot | Req | Svc |    Service Data     |  Redund |  CRC    |
// |             |           | Ver  | ACK | ID  |                     |   Info  |         |
//  ---------------------------------------------------------------------------------------
// | Contents    | I   O   T |   1  | 0/1 | 0-F |   0 - FFFFF         |  Random |  0-FF   |
//  ---------------------------------------------------------------------------------------

// Set the header of the string to 'IoT'
#define SET_HEADER(stringArray) { \
    stringArray[0] = 'I'; \
    stringArray[1] = 'o'; \
    stringArray[2] = 'T'; \
}

// Returns 1 if the header matches
#define IS_HEADER_VALID(array) (strncmp(array, "IoT", 3) ? 0 : 1)

#define SET_PROTOCOL_VER(array, version) { \
    array[3] = (char)version + 0x30; \
}

#define GET_PROTOCOL_VER(array, version) { \
    version = (int)array[3] - 0x30; \
}

#define SET_REQ_ACK(array, value) { \
    array[4] = (char)value + 0x30; \
}

#define GET_REQ_ACK(array, value) { \
    value = (int)array[4] - 0x30; \
}

#define SET_SERVICE_ID(array, serviceID) { \
    array[5] = (char)serviceID + 0x30; \
}

#define GET_SERVICE_ID(array, serviceID) { \
    serviceID = (int)array[5] - 0x30; \
}

#define SET_SERVICE_DATA(array, serviceData) { \
    array[6] = ((serviceData & 0xFF00000000) >> 32) + 0x30; \
    array[7] = ((serviceData & 0xFF000000) >> 24) + 0x30; \
    array[8] = ((serviceData & 0xFF0000) >> 16) + 0x30; \
    array[9] = ((serviceData & 0xFF00) >> 8) + 0x30; \
    array[10] = ((serviceData & 0xFF) ) + 0x30; \
}

#define GET_SERVICE_DATA(array, serviceData) { \
    serviceData = (int)(((array[6]) - 0x30)<< 24); \
    serviceData |= (int)(((array[7]) - 0x30) << 24); \
    serviceData |= (int)(((array[8]) - 0x30) << 16); \
    serviceData |= (int)(((array[9]) -0x30)<< 8); \
    serviceData |= (int)array[10] - 0x30; \
}

#define GET_SALT(array, salt) { \
    salt[0] = array[6]; \
    salt[1] = array[7]; \
    salt[2] = array[8]; \
    salt[3] = array[9]; \
    salt[4] = array[10]; \
    salt[5] = '\0'; \
}


#define SET_REDUNDANT_INFO(array) { \
    sprintf(&array[11], "%02x", rand() % 0xff); \
}

// TODO - Change this
#define SET_CRC(array, crcValue) { \
    sprintf(&array[13], "%02x", crcValue); \
}

#define GET_CRC(array, crcValue) { \
    crcValue = (int)ASCII_TO_BCD(&array[13]); \
}

#endif //IOT_TASK_3_PROTOCOL_H
