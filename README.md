# IoT - Task 3 Group Assignment

## Group Members

- Jonathan White (ID: 93003016)
- Gwyn Wilkinson (ID: 09028091)


## Description

<center>![image](/uploads/301b3ff6ada6cd74dead2339fae8a1ca/image.png)</center>

This assignment implements a simple authentication system that provides pin protected device access and encrypted data communication between an Android phone and a Micro:bit, using Bluetooth LE.

The sending device will be a custom Android app which will utilise the BLE UART service to send encrypted commands to the Micro:bit using a protocol message structure.

The Micro:bit will receive the encrypted text and then prompt the user for a pin code input. The pin code and the per-session salt will be hashed using SHA-1 and used as the decryption key for the received message.

## Code Locations

Android Client code respository:- https://gitlab.uwe.ac.uk/jd6-white/IoT-Task-3-Android-Code

Micro:bit code respository:- https://gitlab.uwe.ac.uk/jd6-white/iot_task_3

## How to build

Clone the entire repository

   git clone https://gitlab.uwe.ac.uk/jd6-white/iot_task_3.git
    
Build the code for the microbit

    yt build

## How to run

Copy the file to the microbit:

    cp build/bbc-microbit-classic-gcc/source/iot-task-3-combined.hex /media/<user>/MICROBIT

To read the serial device outupt:

    sudo cat /dev/ttyACM0
    
Use the Android client to send encrypted commands to the Micro:bit. When prompted for the 3-digit PIN entry on the Micro:bit, use Button-A to change the PIN value, and Button-B to select the digit value.

## Micro:bit Code Description

The following files implement the Micro:bit solution:

 - **main.cpp** - Main program entry. Defines BT and Button Handlers.
 - **aes-128.cpp** - Contains the AES-128-ECB functions
 - **ccitt-crc.cpp** - Defines the CRC functions
 - **communication.cpp** - Handles the inter-device communication functions as as the function to generate the DPK, decrypt the message and create and send the acknowledgement message back to the client.
 - **services.cpp** - Functions to handle the LED, FAN, BUZZER and RGB LED services
 - **sha1.cpp** - Defines the functions to perform the SHA-1 Hash used in DPK creation.
 - **utility.cpp** - General utility function definitions for ASCII_TO_BCD and BCD_TO_ASCII conversion
 - **protocol.h** - Header file which defines the protocol message structure along with Get and Set macros for manipulating the message.
 