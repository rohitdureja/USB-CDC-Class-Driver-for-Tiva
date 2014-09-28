USB-CDC-Class-Driver-for-Tiva
=============================

An API to use the USB CDC device class on Tiva microcontrollers for serial port communication.

How to Use the API
------------------

1. In your main.c, include usbconfig.h and usb_structs.h
2. Create a RxDataHandler() function in your main.c which handles all data received on the USB RX channel.
3. In your main() function, make a call to USBInit() to put the USB device on the bus.

Useful Functions
-------------

1. USBBufferDataAvailable() - This returns the number of bytes in the RX buffer which are waiting to be read.
2. USBBufferRead() - This function reads a defined number of bytes and stores them in an application buffer.
3. USBTxBuffer() - This function places a defined number of bytes from an application array in to the TX buffer.

Refer to the Tiva Peripheral Driver User Guide for information regarding use of these functions and many other functions.

Important Note
-------------

1. The name of the RX and TX buffer to be used in your application code is RxBuffer and TxBuffer respectively. If you want to change these names, it can be done in the usb_struct.h header file.