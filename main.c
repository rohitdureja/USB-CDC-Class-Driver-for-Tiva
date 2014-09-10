

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"

//*****************************************************************************
//
// Define pin to LED color mapping.
//
//*****************************************************************************

#define LED_0 GPIO_PIN_4

//*****************************************************************************
//
//! \addtogroup example_list
//! <h1>Project Zero (project0)</h1>
//!
//! This example demonstrates the use of TivaWare to setup the clocks and
//! toggle GPIO pins to make the LED's blink. This is a good place to start
//! understanding your launchpad and the tools that can be used to program it.
//! See http://www.ti.com/tm4c123g-launchpad/project0 for more information and
//! tutorial videos.
//!
//
//*****************************************************************************


//*****************************************************************************
//
// The error routine that is called if the driver library encounters an error.
//
//*****************************************************************************
#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
}
#endif

//*****************************************************************************
//
// Main 'C' Language entry point.  Toggle an LED using TivaWare.
// See http://www.ti.com/tm4c123g-launchpad/project0 for more information and
// tutorial videos.
//
//*****************************************************************************
int
main(void)
{
    // Setup the system clock to run at 12.5 Mhz from PLL with internal oscillator and disable main oscillator
    SysCtlClockSet(SYSCTL_SYSDIV_16 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);

    //
    // Enable and configure the GPIO port for the LED operation.
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
    GPIOPinTypeGPIOOutput(GPIO_PORTC_BASE, LED_0 );

    //
    // Loop Forever
    //
    while(1)
    {
    	GPIOPinWrite(GPIO_PORTC_BASE, LED_0, 0);
    	SysCtlDelay(SysCtlClockGet()/12);
    	GPIOPinWrite(GPIO_PORTC_BASE, LED_0, LED_0);
    	SysCtlDelay(SysCtlClockGet()/12);
    }
}
