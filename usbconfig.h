/*
 * usbconfig.h
 *
 *  Created on: Sep 27, 2014
 *      Author: Lab
 */

#ifndef USBCONFIG_H_
#define USBCONFIG_H_

// Defines required to redirect UART0 via USB.
#define USB_UART_BASE           UART0_BASE
#define USB_UART_PERIPH         SYSCTL_PERIPH_UART0
#define USB_UART_INT            INT_UART0

// Flags used to pass commands from interrupt context to the main loop.
#define COMMAND_PACKET_RECEIVED 0x00000001
#define COMMAND_STATUS_UPDATE   0x00000002

volatile uint32_t g_ui32Flags;
char *g_pcStatus;

// Global flag indicating that a USB configuration has been set.
static volatile bool g_bUSBConfigured;// = false;

// Internal function prototypes.
static void SetControlLineState(uint16_t ui16State);
static bool SetLineCoding(tLineCoding *psLineCoding);
static void GetLineCoding(tLineCoding *psLineCoding);
void USBInit(void);
extern void RxDataHandler(void);

#endif /* USBCONFIG_H_ */
