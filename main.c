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

// Define pin to LED color mapping.
#define LED_0 GPIO_PIN_1

// Internal function prototypes
void ConfigureUART(void);
static void CheckForSerialStateChange(const tUSBDCDCDevice *psDevice, int32_t i32errors);
static void SetControlLineState(uint16_t ui16State);
static bool SetLineCoding(tLineCoding *psLineCoding);
static void GetLineCoding(tLineCoding *psLineCoding);
static void SendBreak(bool bSend);

// Global flag indicating that a USB configuration is set
static volatile bool USBConfigured = false;
// Handles CDC driver notifications related to the receive channel (data from USB host)
uint32_t RxHandler(void *pvCBData, uint32_t ui32Event, uint32_t ui32MsgValue, void *pvMsgData)
{
	switch(ui32Event)
	{
		// A new packet has been received
		case USB_EVENT_RX_AVAILABLE:
		{
			break;
		}
		// Being asked if we are still processing data.
		case USB_EVENT_DATA_REMAINING:
		{
			break;
		}
		case USB_EVENT_ERROR:
		{
			break;
		}
		default:
			break;
	}
	return 0;
}

// Handles CDC driver notifications related to the transmit channel (data to USB host)
uint32_t TxHandler(void *pvCBData, uint32_t ui32Event, uint32_t ui32MsgValue, void *pvMsgData)
{
	switch(ui32Event)
	{
		case USB_EVENT_TX_COMPLETE:
			// Since we using USB buffers, we don't need to do anything here
			break;
		default:
			break;
	}
	return 0;
}

// Handles CDC driver notifications related to control and setup of the device.
uint32_t ControlHandler(void *pvCBData, uint32_t ui32Event, uint32_t ui32MsgData, void *pvMsgData)
{
	uint32_t ui32IntsOff;

	switch(ui32Event)
	{
		case USB_EVENT_CONNECTED:
		{
			USBConfigured = true;
			USBBufferFlush(&g_sTxBuffer);
			USBBufferFlush(&g_sRxBuffer);
			ui32IntsOff = ROM_IntMasterDisable();
			UARTprintf("\nUSB now connected as device");
			if(!ui32IntsOff)
			{
				ROM_IntMasterEnable();
			}
			break;
		}
		case USB_EVENT_DISCONNECTED:
		{
			USBConfigured = false;
			ui32IntsOff = ROM_IntMasterDisable();
			UARTprintf("\nUSB now disconnected as device");
			if(!ui32IntsOff)
			{
				ROM_IntMasterEnable();
			}
			break;
		}
		default:
			break;
	}
	return 0;
}


// UART configuration for uartstdio library
void ConfigureUART(void)
{
    // Enable the GPIO Peripheral used by the UART.
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    // Enable UART0
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    // Configure GPIO Pins for UART mode.
    ROM_GPIOPinConfigure(GPIO_PA0_U0RX);
    ROM_GPIOPinConfigure(GPIO_PA1_U0TX);
    ROM_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    // Use the internal 16MHz oscillator as the UART clock source.
    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);

    // Initialize the UART for console I/O.
    UARTStdioConfig(0, 115200, 16000000);
}

int main(void)
{
    // Enable lazy stacking
	ROM_FPULazyStackingEnable();

	// Set clocking to run at 50MHz
	ROM_SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);

	// Configure the required pins for USB operation
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
	ROM_GPIOPinTypeUSBAnalog(GPIO_PORTD_BASE, GPIO_PIN_5 | GPIO_PIN_4);

	// Enable GPIO pin used for onboard LED
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

	// Enable the GPIO pins for the LED
	ROM_GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_3 | GPIO_PIN_2);

	// configure UART for uartstdio
	ConfigureUART();

	// USB not configured initially
	USBConfigured = false;

	// Initialize the transmit and receive buffers
	USBBufferInit(&g_sTxBuffer);
	USBBufferInit(&g_sRxBuffer);

	// Set the USB stack mode to device mode
	USBStackModeSet(0, eUSBModeDevice, 0);

	USBDCDCInit(0, &g_sCDCDevice);

	UARTprintf("\nConnecting as device");

	// Loop forever
	while(1)
	{

	}
}
