#ifndef IOT_TASK_3_PROTOCOL_H
#define IOT_TASK_3_PROTOCOL_H

// Defines for protocol elements
# define PROTOCOL_VERSION 1

# define REQUEST 0
# define ACKNOWLEDGE 1

// Defines for service IDs and Data

// TODO - defines here

// Protocol Message definition

//  -------------------------------------------------------------------------------------------
// | Byte        | 0 | 1 | 2 |  3  |   4  | 5   | 6 | 7 | 8 |  9 | 10 | 11 | 12 | 13 | 14 | 15 |
//  -------------------------------------------------------------------------------------------
// | Description |   Header  | Prot | Req | Svc |    Service Data          |  Redund |   CRC   |
// |             |           | Ver  | ACK | ID  |                          |   Info  |         |
//  -------------------------------------------------------------------------------------------
// | Contents    | I   O   T |   1  | 0/1 | 0-FF|   0 - FFFFFFFFFFFF       |  Random |  0-FFFF |
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
    version = (int)array[3]; \
}

#define SET_REQ_ACK(array, value) { \
    array[4] = (char)value; \
}

#define GET_REQ_ACK(array, value) { \
    value = (int)array[4]; \
}

#define SET_SERVICE_ID(array, serviceID) { \
    array[5] = (char)serviceID; \
}

#define GET_SERVICE_ID(array, serviceID) { \
    serviceID = (int)array[5]; \
}

#define SET_SERVICE_DATA(array, serviceData) { \
    array[6] = ((serviceData & 0xFF0000000000) >> 40); \
    array[7] = ((serviceData & 0xFF00000000) >> 32); \
    array[8] = ((serviceData & 0xFF000000) >> 24); \
    array[9] = ((serviceData & 0xFF0000) >> 16); \
    array[10] = ((serviceData & 0xFF00) >> 8); \
    array[11] = (serviceData & 0xFF); \
}

#define GET_SERVICE_DATA(array, serviceData) { \
    serviceData |= (long long)((array[6])) << 40; \
    serviceData |= (long long)((array[7])) << 32; \
    serviceData |= (long long)((array[8])) << 24; \
    serviceData |= (long long)((array[9])) << 16; \
    serviceData |= (long long)((array[10])) << 8; \
    serviceData |= (long long)array[11]; \
}

#define SET_REDUNDANT_INFO(array, randomNumber) { \
    array[12] = ((randomNumber & 0xFF00) >> 8); \
    array[13] = (randomNumber & 0xFF); \
}

#define SET_CRC(array, crcValue) { \
    array[14] = ((crcValue & 0xFF00) >> 8); \
    array[15] = (crcValue & 0xFF); \
}

#define GET_CRC(array, crcValue) { \
    crcValue = array[14] << 8; \
    crcValue |= array[15]; \
}

#endif //IOT_TASK_3_PROTOCOL_H
