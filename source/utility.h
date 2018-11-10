#ifndef IOT_TASK_3_UTILTIY_H
#define IOT_TASK_3_UTILTIY_H

// Utility Macro to convert two ASCII characters to Hex BCD format
unsigned char ASCII_TO_BCD(char ascii_text[2])
{
    unsigned char bcd_value;

    // left side
    if(ascii_text[0] >= '0' && ascii_text[0] <= '9')  // 0-9 range
    {
        bcd_value = ( ascii_text[0] - 48)  << 4 ; // 48 for '0' ASCII offset
    }
    else if (ascii_text[0] >= 'A' && ascii_text[0] <= 'F') // A-F range
    {
        bcd_value = ( 10 + ascii_text[0] - 65 )  << 4 ; // 65 for 'A' ASCII offset
    }
    else if (ascii_text[0] >= 'a' && ascii_text[0] <= 'f') // a-f range
    {
        bcd_value = ( 10 + ascii_text[0] - 97)  << 4 ; // 97 for 'a'  ASCII offset
    }

    // right side
    if(ascii_text[1] >= '0' && ascii_text[1] <= '9')  // 0-9 range
    {
        bcd_value |= ( ascii_text[1] - 48); // 48 for '0' ASCII offset
    }
    else if (ascii_text[1] >= 'A' && ascii_text[1] <= 'F') // A-F range
    {
        bcd_value |= ( 10 + ascii_text[1] - 65)   ; // 65 for 'A' ASCII offset
    }
    else if (ascii_text[1] >= 'a' && ascii_text[1] <= 'f') // a-f range
    {
        bcd_value |= ( 10 + ascii_text[1] - 97 ) ; // 97 for 'a' ASCII offset
    }

    return bcd_value;
}

#endif //IOT_TASK_3_UTILTIY_H
