USB-CDC-Class-Driver-for-Tiva
=============================

An API to use the USB CDC device class on Tiva microcontrollers for serial port communication.

How to Use the API
------------------

1. In your main.c, include usbconfig.h and usb_structs.h
2. Create a RxDataHandler() function in your main.c which handles all data received on the USB RX channel.