/*
 * usbconfig.c
 *
 *  Created on: Sep 27, 2014
 *      Author: Lab
 */

#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_uart.h"
#include "inc/hw_sysctl.h"
#include "driverlib/debug.h"
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/timer.h"
#include "driverlib/uart.h"
#include "driverlib/usb.h"
#include "driverlib/rom.h"
#include "usblib/usblib.h"
#include "usblib/usbcdc.h"
#include "usblib/usb-ids.h"
#include "usblib/device/usbdevice.h"
#include "usblib/device/usbdcdc.h"
#include "utils/ustdlib.h"
#include "usb_structs.h"
#include "utils/uartstdio.h"
#include "usbconfig.h"

// Initialise the USB peripheral
void USBInit(void)
{
	g_ui32Flags = 0;
	g_bUSBConfigured = false;

	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
	ROM_GPIOPinTypeUSBAnalog(GPIO_PORTD_BASE, GPIO_PIN_5 | GPIO_PIN_4);

	// Initialize the transmit and receive buffers.
	USBBufferInit(&TxBuffer);
	USBBufferInit(&RxBuffer);

	// Set the USB stack mode to Device mode with VBUS monitoring.
	USBStackModeSet(0, eUSBModeForceDevice, 0);

	// Pass our device information to the USB library and place the device on the bus.
	USBDCDCInit(0, &g_sCDCDevice);
}


// Set the state of the RS232 RTS and DTR signals.
static void SetControlLineState(uint16_t ui16State)
{
    //
    // TODO: If configured with GPIOs controlling the handshake lines,
    // set them appropriately depending upon the flags passed in the wValue
    // field of the request structure passed.
    //
}

// Set the communication parameters to use on the UART.
static bool SetLineCoding(tLineCoding *psLineCoding)
{
    uint32_t ui32Config;
    bool bRetcode;

    // Assume everything is OK until we detect any problem.
    bRetcode = true;

    // Word length.  For invalid values, the default is to set 8 bits per
    // character and return an error.
    switch(psLineCoding->ui8Databits)
    {
        case 5:
        {
            ui32Config = UART_CONFIG_WLEN_5;
            break;
        }
        case 6:
        {
            ui32Config = UART_CONFIG_WLEN_6;
            break;
        }
        case 7:
        {
            ui32Config = UART_CONFIG_WLEN_7;
            break;
        }
        case 8:
        {
            ui32Config = UART_CONFIG_WLEN_8;
            break;
        }
        default:
        {
            ui32Config = UART_CONFIG_WLEN_8;
            bRetcode = false;
            break;
        }
    }

    // Parity.  For any invalid values, we set no parity and return an error.
    switch(psLineCoding->ui8Parity)
    {
        case USB_CDC_PARITY_NONE:
        {
            ui32Config |= UART_CONFIG_PAR_NONE;
            break;
        }
        case USB_CDC_PARITY_ODD:
        {
            ui32Config |= UART_CONFIG_PAR_ODD;
            break;
        }
        case USB_CDC_PARITY_EVEN:
        {
            ui32Config |= UART_CONFIG_PAR_EVEN;
            break;
        }
        case USB_CDC_PARITY_MARK:
        {
            ui32Config |= UART_CONFIG_PAR_ONE;
            break;
        }
        case USB_CDC_PARITY_SPACE:
        {
            ui32Config |= UART_CONFIG_PAR_ZERO;
            break;
        }
        default:
        {
            ui32Config |= UART_CONFIG_PAR_NONE;
            bRetcode = false;
            break;
        }
    }

    // Stop bits.  Our hardware only supports 1 or 2 stop bits whereas CDC
    // allows the host to select 1.5 stop bits.  If passed 1.5 (or any other
    // invalid or unsupported value of ui8Stop, we set up for 1 stop bit but
    // return an error in case the caller needs to Stall or otherwise report
    // this back to the host.
    switch(psLineCoding->ui8Stop)
    {
        // One stop bit requested.
        case USB_CDC_STOP_BITS_1:
        {
            ui32Config |= UART_CONFIG_STOP_ONE;
            break;
        }
        // Two stop bits requested.
        case USB_CDC_STOP_BITS_2:
        {
            ui32Config |= UART_CONFIG_STOP_TWO;
            break;
        }
        // Other cases are either invalid values of ui8Stop or values that we
        // cannot support so set 1 stop bit but return an error.
        default:
        {
            ui32Config |= UART_CONFIG_STOP_ONE;
            bRetcode = false;
            break;
        }
    }
    // Let the caller know if we had a problem or not.
    return(bRetcode);
}

// Get the communication parameters in use on the UART.
static void GetLineCoding(tLineCoding *psLineCoding)
{
    uint32_t ui32Config;
    uint32_t ui32Rate;

    // Get the current line coding set in the UART.
    ROM_UARTConfigGetExpClk(USB_UART_BASE, ROM_SysCtlClockGet(), &ui32Rate,
                            &ui32Config);
    psLineCoding->ui32Rate = ui32Rate;

    // Translate the configuration word length field into the format expected
    // by the host.
    switch(ui32Config & UART_CONFIG_WLEN_MASK)
    {
        case UART_CONFIG_WLEN_8:
        {
            psLineCoding->ui8Databits = 8;
            break;
        }
        case UART_CONFIG_WLEN_7:
        {
            psLineCoding->ui8Databits = 7;
            break;
        }
        case UART_CONFIG_WLEN_6:
        {
            psLineCoding->ui8Databits = 6;
            break;
        }
        case UART_CONFIG_WLEN_5:
        {
            psLineCoding->ui8Databits = 5;
            break;
        }
    }

    // Translate the configuration parity field into the format expected
    // by the host.
    switch(ui32Config & UART_CONFIG_PAR_MASK)
    {
        case UART_CONFIG_PAR_NONE:
        {
            psLineCoding->ui8Parity = USB_CDC_PARITY_NONE;
            break;
        }
        case UART_CONFIG_PAR_ODD:
        {
            psLineCoding->ui8Parity = USB_CDC_PARITY_ODD;
            break;
        }
        case UART_CONFIG_PAR_EVEN:
        {
            psLineCoding->ui8Parity = USB_CDC_PARITY_EVEN;
            break;
        }
        case UART_CONFIG_PAR_ONE:
        {
            psLineCoding->ui8Parity = USB_CDC_PARITY_MARK;
            break;
        }
        case UART_CONFIG_PAR_ZERO:
        {
            psLineCoding->ui8Parity = USB_CDC_PARITY_SPACE;
            break;
        }
    }

    // Translate the configuration stop bits field into the format expected
    // by the host.
    switch(ui32Config & UART_CONFIG_STOP_MASK)
    {
        case UART_CONFIG_STOP_ONE:
        {
            psLineCoding->ui8Stop = USB_CDC_STOP_BITS_1;
            break;
        }

        case UART_CONFIG_STOP_TWO:
        {
            psLineCoding->ui8Stop = USB_CDC_STOP_BITS_2;
            break;
        }
    }
}

//*****************************************************************************
//
// Handles CDC driver notifications related to control and setup of the device.
//
// \param pvCBData is the client-supplied callback pointer for this channel.
// \param ui32Event identifies the event we are being notified about.
// \param ui32MsgValue is an event-specific value.
// \param pvMsgData is an event-specific pointer.
//
// This function is called by the CDC driver to perform control-related
// operations on behalf of the USB host.  These functions include setting
// and querying the serial communication parameters, setting handshake line
// states and sending break conditions.
//
// \return The return value is event-specific.
//
//*****************************************************************************
uint32_t ControlHandler(void *pvCBData, uint32_t ui32Event,
               uint32_t ui32MsgValue, void *pvMsgData)
{
    uint32_t ui32IntsOff;

    // Which event are we being asked to process?
    switch(ui32Event)
    {
        // We are connected to a host and communication is now possible.
        case USB_EVENT_CONNECTED:
            g_bUSBConfigured = true;

            // Flush our buffers.
            USBBufferFlush(&TxBuffer);
            USBBufferFlush(&RxBuffer);

            // Tell the main loop to update the display.
            ui32IntsOff = ROM_IntMasterDisable();
            g_pcStatus = "Connected";
            g_ui32Flags |= COMMAND_STATUS_UPDATE;
            if(!ui32IntsOff)
            {
                ROM_IntMasterEnable();
            }
            break;
        // The host has disconnected.
        case USB_EVENT_DISCONNECTED:
            g_bUSBConfigured = false;
            ui32IntsOff = ROM_IntMasterDisable();
            g_pcStatus = "Disconnected";
            g_ui32Flags |= COMMAND_STATUS_UPDATE;
            if(!ui32IntsOff)
            {
                ROM_IntMasterEnable();
            }
            break;
        // Return the current serial communication parameters.
        case USBD_CDC_EVENT_GET_LINE_CODING:
            GetLineCoding(pvMsgData);
            break;
        // Set the current serial communication parameters.
        case USBD_CDC_EVENT_SET_LINE_CODING:
            SetLineCoding(pvMsgData);
            break;
        // Set the current serial communication parameters.
        case USBD_CDC_EVENT_SET_CONTROL_LINE_STATE:
            SetControlLineState((uint16_t)ui32MsgValue);
            break;
        // Send a break condition on the serial line.
        case USBD_CDC_EVENT_SEND_BREAK:
            break;
        // Clear the break condition on the serial line.
        case USBD_CDC_EVENT_CLEAR_BREAK:
            break;
        // Ignore SUSPEND and RESUME for now.
        case USB_EVENT_SUSPEND:
        case USB_EVENT_RESUME:
            break;
        // We don't expect to receive any other events.  Ignore any that show
        // up in a release build or hang in a debug build.
        default:
#ifdef DEBUG
            while(1);
#else
            break;
#endif
    }
    return(0);
}

//*****************************************************************************
//
// Handles CDC driver notifications related to the transmit channel (data to
// the USB host).
//
// \param ui32CBData is the client-supplied callback pointer for this channel.
// \param ui32Event identifies the event we are being notified about.
// \param ui32MsgValue is an event-specific value.
// \param pvMsgData is an event-specific pointer.
//
// This function is called by the CDC driver to notify us of any events
// related to operation of the transmit data channel (the IN channel carrying
// data to the USB host).
//
// \return The return value is event-specific.
//
//*****************************************************************************
uint32_t TxHandler(void *pvCBData, uint32_t ui32Event, uint32_t ui32MsgValue,
          void *pvMsgData)
{
    // Which event have we been sent?
    switch(ui32Event)
    {
        case USB_EVENT_TX_COMPLETE:
            // Since we are using the USBBuffer, we don't need to do anything
            // here.
            break;
        // We don't expect to receive any other events.  Ignore any that show
        // up in a release build or hang in a debug build.
        default:
#ifdef DEBUG
            while(1);
#else
            break;
#endif
    }
    return(0);
}

//*****************************************************************************
//
// Handles CDC driver notifications related to the receive channel (data from
// the USB host).
//
// \param ui32CBData is the client-supplied callback data value for this channel.
// \param ui32Event identifies the event we are being notified about.
// \param ui32MsgValue is an event-specific value.
// \param pvMsgData is an event-specific pointer.
//
// This function is called by the CDC driver to notify us of any events
// related to operation of the receive data channel (the OUT channel carrying
// data from the USB host).
//
// \return The return value is event-specific.
//
//*****************************************************************************
uint32_t RxHandler(void *pvCBData, uint32_t ui32Event, uint32_t ui32MsgValue,
          void *pvMsgData)
{
    uint32_t ui32Count;

    // Which event are we being sent?
    switch(ui32Event)
    {
        // A new packet has been received.
        case USB_EVENT_RX_AVAILABLE:
        {
            // Call the user defined RX data handler
        	RxDataHandler();
            break;
        }
        // We are being asked how much unprocessed data we have still to
        // process. We return 0 if the UART is currently idle or 1 if it is
        // in the process of transmitting something. The actual number of
        // bytes in the UART FIFO is not important here, merely whether or
        // not everything previously sent to us has been transmitted.
        case USB_EVENT_DATA_REMAINING:
        {
            // Get the number of bytes in the buffer and add 1 if some data
            // still has to clear the transmitter.
            ui32Count = ROM_UARTBusy(USB_UART_BASE) ? 1 : 0;
            return(ui32Count);
        }
        // We are being asked to provide a buffer into which the next packet
        // can be read. We do not support this mode of receiving data so let
        // the driver know by returning 0. The CDC driver should not be sending
        // this message but this is included just for illustration and
        // completeness.
        case USB_EVENT_REQUEST_BUFFER:
        {
            return(0);
        }
        // We don't expect to receive any other events.  Ignore any that show
        // up in a release build or hang in a debug build.
        default:
#ifdef DEBUG
            while(1);
#else
            break;
#endif
    }
    return(0);
}
