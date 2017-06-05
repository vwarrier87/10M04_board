//*****************************************************************************
//
// usb_dev_bulk.c - Main routines for the generic bulk device example.
//
// Copyright (c) 2012-2015 Texas Instruments Incorporated.  All rights reserved.
// Software License Agreement
// 
// Texas Instruments (TI) is supplying this software for use solely and
// exclusively on TI's microcontroller products. The software is owned by
// TI and/or its suppliers, and is protected under applicable copyright
// laws. You may not combine this software with "viral" open-source
// software in order to form a larger program.
// 
// THIS SOFTWARE IS PROVIDED "AS IS" AND WITH ALL FAULTS.
// NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT
// NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. TI SHALL NOT, UNDER ANY
// CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
// DAMAGES, FOR ANY REASON WHATSOEVER.
// 
// This is part of revision 2.1.1.71 of the EK-TM4C123GXL Firmware Package.
//
//*****************************************************************************

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/timer.h"
#include "driverlib/uart.h"
#include "driverlib/rom.h"
#include "usblib/usblib.h"
#include "usblib/usb-ids.h"
#include "usblib/device/usbdevice.h"
#include "usblib/device/usbdbulk.h"
#include "utils/uartstdio.h"
#include "utils/ustdlib.h"
#include "usb_bulk_structs.h"

//*****************************************************************************
//
//! \addtogroup example_list
//! <h1>USB Generic Bulk Device (usb_dev_bulk)</h1>
//!
//! This example provides a generic USB device offering simple bulk data
//! transfer to and from the host.  The device uses a vendor-specific class ID
//! and supports a single bulk IN endpoint and a single bulk OUT endpoint.
//! Data received from the host is assumed to be ASCII text and it is
//! echoed back with the case of all alphabetic characters swapped.
//!
//! A Windows INF file for the device is provided on the installation CD and
//! in the C:/ti/TivaWare-for-C-Series/windows_drivers directory of TivaWare C
//! series releases.  This INF contains information required to install the
//! WinUSB subsystem on Windowi16XP and Vista PCs.  WinUSB is a Windows
//! subsystem allowing user mode applications to access the USB device without
//! the need for a vendor-specific kernel mode driver.
//!
//! A sample Windows command-line application, usb_bulk_example, illustrating
//! how to connect to and communicate with the bulk device is also provided.
//! The application binary is installed as part of the ''Windows-side examples
//! for USB kits'' package (SW-USB-win) on the installation CD or via download
//! from http://www.ti.com/tivaware .  Project files are included to allow
//! the examples to be built using Microsoft VisualStudio 2008.  Source code
//! for this application can be found in directory
//! TivaWare-for-C-Series/tools/usb_bulk_example.
//
//*****************************************************************************

//*****************************************************************************
//
// The system tick rate expressed both as ticks per second and a millisecond
// period.
//
//*****************************************************************************
#define SYSTICKS_PER_SECOND     100
#define SYSTICK_PERIOD_MS       (1000 / SYSTICKS_PER_SECOND)
#define BULK_Byte_SIZE 12
//*****************************************************************************
//
// The global system tick counter.
//
//*****************************************************************************
volatile uint32_t g_ui32SysTickCount = 0;

//*****************************************************************************
//
// Variables tracking transmit and receive counts.
//
//*****************************************************************************
volatile uint32_t g_ui32TxCount = 0;
volatile uint32_t g_ui32RxCount = 0;
#ifdef DEBUG
uint32_t g_ui32UARTRxErrors = 0;
#endif

//*****************************************************************************
//
// Debug-related definitions and declarations.
//
// Debug output is available via UART0 if DEBUG is defined during build.
//
//*****************************************************************************
#ifdef DEBUG
//*****************************************************************************
//
// Map all debug print calls to UARTprintf in debug builds.
//
//*****************************************************************************
#define DEBUG_PRINT UARTprintf

#else

//*****************************************************************************
//
// Compile out all debug print calls in release builds.
//
//*****************************************************************************
#define DEBUG_PRINT while(0) ((int (*)(char *, ...))0)
#endif

//*****************************************************************************
//
// Flags used to pass commands from interrupt context to the main loop.
//
//*****************************************************************************
#define COMMAND_PACKET_RECEIVED 0x00000001
#define COMMAND_STATUS_UPDATE   0x00000002

volatile uint32_t g_ui32Flags = 0;

//*****************************************************************************
//
// Global flag indicating that a USB configuration has been set.
//
//*****************************************************************************
static volatile bool g_bUSBConfigured = false;

//*****************************************************************************
//
// The error routine that is called if the driver library encounters an error.
//
//*****************************************************************************
#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
    UARTprintf("Error at line %d of %s\n", ui32Line, pcFilename);
    while(1)
    {
    }
}
#endif

//*****************************************************************************
//
// Interrupt handler for the system tick counter.
//
//*****************************************************************************
void
SysTickIntHandler(void)
{
    //
    // Update our system tick counter.
    //
    g_ui32SysTickCount++;
}

//*****************************************************************************
//
// Receive new data and echo it back to the host.
//
// \param psDevice points to the instance data for the device whose data is to
// be processed.
// \param pui8Data points to the newly received data in the USB receive buffer.
// \param ui32NumBytes is the number of bytes of data available to be processed.
//
// This function is called whenever we receive a notification that data is
// available from the host. We read the data, byte-by-byte and swap the case
// of any alphabetical characters found then write it back out to be
// transmitted back to the host.
//
// \return Returns the number of bytes of data processed.
//
//*****************************************************************************
void Sys_config_cpld(void)
{

    /*TDI, LOW = GPIO_PORTB_BASE GPIO_PIN_0
	TRST, HIGH                   GPIO_PIN_4
	TMS, LOW                     GPIO_PIN_1
	TCLK, LOW      GPIO_PORTA_BASE              GPIO_PIN_5
     TD0       = GPIO_PORTB_BASE GPIO_PIN_5
     */

	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_4 | GPIO_PIN_1| GPIO_PIN_0);
    GPIOPinTypeGPIOInput(GPIO_PORTB_BASE, GPIO_PIN_5);
    GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE, GPIO_PIN_5);
    GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_4 | GPIO_PIN_1| GPIO_PIN_0, 16);
    GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_5,0);
}

void delay()
{ volatile uint32_t ui32delay;
for(ui32delay = 0; ui32delay < 15; ui32delay++)
        {
        }
}

void delay_more()
{ volatile uint32_t ui32delay_more,ui32delay_more2;
for(ui32delay_more = 0; ui32delay_more < 150000; ui32delay_more++)
        {
	for(ui32delay_more2 = 0; ui32delay_more2 < 5; ui32delay_more2++);
        }
}
void applyTMS(int TMS)
{
	GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_1,TMS);
	GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_5,0);
	delay();
	GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_5,32);
	delay();
}


uint8_t scan_out_in(int TDI)
{
	uint8_t TDO_read = (uint8_t)GPIOPinRead(GPIO_PORTB_BASE, GPIO_PIN_5);
	GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_0,TDI);
	GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_1,0);
	GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_5,0);
	delay();
	GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_5,32);
	delay();
	return TDO_read;
}

void applyTDI(int TDI)
{
	GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_0,TDI);
	GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_1,0);
	GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_5,0);
	delay();
	GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_5,32);
	delay();
}

#define TMS 0x02		//Corresponds to a 1 at pin B1
#define TDI 0x01		//Corresponds to a 1 at pin B0
#define TRST 0x10		//Corresponds to a 1 at pin B4

uint8_t JTAG_clock(uint32_t val)
{
	GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_4, val);
	GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_5,0);
	//Delay of 10us
	//SysCtlDelay(SysCtlClockGet()/10000);
	//GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_5,32);
	//delay();
	//Delay of 10us
	//SysCtlDelay(SysCtlClockGet()/10000);
	//delay();
	
	delay();
	GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_5,32);
	delay();

	if(GPIOPinRead(GPIO_PORTB_BASE, GPIO_PIN_5) == GPIO_PIN_5)
		return 1;
	else
		return 0;
}

uint8_t JTAG_read()
{
    GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_5,0);
    //Delay of 10us
    //SysCtlDelay(SysCtlClockGet()/10000);
    //GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_5,32);
    //Delay of 10us
    //SysCtlDelay(SysCtlClockGet()/10000);
    delay();
    GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_5,32);
    delay();

    if(GPIOPinRead(GPIO_PORTB_BASE, GPIO_PIN_5) == GPIO_PIN_5)
        return 1;
    else
        return 0;
}

char bin_eq[4] = "0000";
void hextobin(uint8_t hexaDecimal)
{
    switch(hexaDecimal)
	 {
        case '0': strcpy(bin_eq,"0000"); break;
        case '1': strcpy(bin_eq,"0001"); break;
        case '2': strcpy(bin_eq,"0010"); break;
        case '3': strcpy(bin_eq,"0011"); break;
        case '4': strcpy(bin_eq,"0100"); break;
        case '5': strcpy(bin_eq,"0101"); break;
        case '6': strcpy(bin_eq,"0110"); break;
        case '7': strcpy(bin_eq,"0111"); break;
        case '8': strcpy(bin_eq,"1000"); break;
        case '9': strcpy(bin_eq,"1001"); break;
        case 'A': strcpy(bin_eq,"1010"); break;
        case 'B': strcpy(bin_eq,"1011"); break;
        case 'C': strcpy(bin_eq,"1100"); break;
        case 'D': strcpy(bin_eq,"1101"); break;
        case 'E': strcpy(bin_eq,"1110"); break;
        case 'F': strcpy(bin_eq,"1111"); break;
        //default:  printf("\nInvalid hexadecimal digit %c n",hexaDecimal);
	}
}

uint8_t temp[BULK_BUFFER_SIZE],array1[12]={'A','B','C','1','0','7','8','9','9','5','8','5'};
uint8_t program_array[50] = {0};
uint32_t program_array2[50] = {0};
uint8_t array2[12]={'W','R','O','N','G','R','I','G','H','T','H','I'};
uint32_t ui32ReadIndex=0,ui32Loop=0;
uint32_t ui32WriteIndex=0, ui32Count=0;
uint8_t state=0;
void read_function()
{
	uint32_t m=0;
	while(ui32Loop)

       {

       	temp[m]=g_pui8USBRxBuffer[ui32ReadIndex];

           m++;
           ui32ReadIndex++;
           ui32ReadIndex = (ui32ReadIndex == BULK_BUFFER_SIZE) ? 0 : ui32ReadIndex;
           m = (m == BULK_BUFFER_SIZE) ? 0 : m;

           ui32Loop--;
       }
}




void write_function(uint8_t *temp2, uint32_t ui32Bytes)
{
	uint32_t byte=ui32Bytes;
	uint32_t n=0;
	while(ui32Bytes)
		{

			g_pui8USBTxBuffer[ui32WriteIndex] =
			temp2[n];

			n++;
			n = (n == byte) ? 0 : n;

			ui32WriteIndex++;
			ui32WriteIndex = (ui32WriteIndex == BULK_BUFFER_SIZE) ?
							 0 : ui32WriteIndex;

			ui32Bytes--;
		}
	USBBufferDataWritten(&g_sTxBuffer, byte);
}
uint8_t temp_in[4], temp_out[21];
int hex_OUTPIN;
static uint32_t
EchoNewDataToHost(tUSBDBulkDevice *psDevice, uint8_t *pui8Data,
                  uint32_t ui32NumBytes)
{
    //uint32_t ui32Space;
    tUSBRingBufObject sTxRing;
    //uint32_t INPIN=0, OUTPIN=0;
    uint32_t scanned_in_bits = 0,scanned_out_bits=0,ui32by;

	uint8_t binary[4]="0000",out_reg[256];
    volatile int k,t,i,hex_INPIN;
    //
    // Get the current buffer information to allow us to write directly to
    // the transmit buffer (we already have enough information from the
    // parameters to access the receive buffer directly).
    //
    USBBufferInfoGet(&g_sTxBuffer, &sTxRing);

    //
    // How much space is there in the transmit buffer?
    //
    //ui32Space = USBBufferSpaceAvailable(&g_sTxBuffer);

    //
    // How many characters can we process this time round?
    //(ui32Space < ui32NumBytes) ? ui32Space :
    ui32Loop = ui32NumBytes;
    ui32Count = ui32Loop;

    //
    // Update our receive counter.
    //
    g_ui32RxCount += ui32NumBytes;

    ui32ReadIndex = (uint32_t)(pui8Data - g_pui8USBRxBuffer);
    ui32WriteIndex = sTxRing.ui32WriteIndex;
    ui32by=15;
    read_function();
    GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_4, 0);


  
if (state==0)
{
	if (temp[0]=='T')
	        {
	        	state=0;
	        	Sys_config_cpld();
	        	ui32by=1;
	        	array1[0]=170;
	        	write_function(array1,ui32by);
	        }

	 else if (temp[0] == 'Z')
	        { 	// end of transaction
				state=0;
				//Sys_config_cpld();
				ui32by=1;
				array1[0]=85;
				write_function(array1,ui32by);
	        }
	else
    if (temp[0]=='L')
    {
    			state=1;
    	        GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_4, 0);
    	        applyTMS(2);
    	        applyTMS(0);
    	        //applyTMS(0);
    }
    else if (temp[0]=='A')
         {
        		state=0;
    			applyTMS(2);
    			applyTMS(2);
         }
    else if (temp[0]=='S')
    {
    			//strcpy(state,"SOUT");
    	        GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_4, 0);
    	        ui32by=temp_in[3];
    	        write_function(out_reg,ui32by);
    }
    else if (temp[0]=='P')
    {
        volatile uint32_t i = 0;

        // go to reset state
        for(i=0; i<5; i++)
            applyTMS(2);

        // go to Shift-IR
        applyTMS(0);
        applyTMS(2);
        applyTMS(2);
        applyTMS(0);
        applyTMS(0);

		//for(i=0; i<2; i++)
		//    applyTDI(1);

		applyTMS(2);  // last bit needs to have TMS active, to exit shift-IR

		// we are in Exit1-IR, go to Shift-DR
		applyTMS(2);
        applyTMS(2);
        applyTMS(0);
        applyTMS(0);

		// Send plenty of zeros into the DR registers to flush them
		for(i=0; i<10; i++)
		    applyTDI(0);

		// now send ones until we receive one back
		for(i=0; i<20; i++)
			if(scan_out_in(1) == 0x20)
				break;
		//UARTprintf("There are %d device(s) in the JTAG chain\n", i);
		program_array[0]=100;
		program_array[1]=(uint8_t)i;
        for(i=2;i<48;i++)
        {
            if(i%2 == 0)
                program_array[i] = scan_out_in(1);
            else
                program_array[i] = scan_out_in(0);
            program_array2[i] = program_array[i];
        }
        write_function(program_array,16);
	}
		
    else if (temp[0]=='I')
    {
        volatile uint32_t i = 0;
        // go to reset state (that loads IDCODE into IR of all the devices)
        for(i=0; i<5; i++)
            applyTMS(2);

        // go to Shift-DR
        applyTMS(0);
        applyTMS(2);
        applyTMS(0);
        applyTMS(0);

        // and read the IDCODES
        ui32by=33;
        program_array[0]=100;
        for(i=1;i<48;i++)
        {
            program_array[i] = scan_out_in(0);
            program_array2[i] = program_array[i];
        }
        write_function(program_array,34);

        //printf("IDCODE for device %d is %08X\n", i+1, JTAG_read(32));

    }
    else if (temp[0]=='Q')
    {
        volatile uint32_t i = 0;
        // go to reset state (that loads IDCODE into IR of all the devices)
        for(i=0; i<5; i++)
            applyTMS(2);

        // go to Shift-IR
        applyTMS(0);
        applyTMS(2);
        applyTMS(2);
        applyTMS(0);
        applyTMS(0);

        // IR is 10 bits long,
        // there is only one device in the chain,
        // and SAMPLE code = 0000000101b   0000000110b
        applyTDI(0);
        applyTDI(1);
        applyTDI(1);
        applyTDI(0);
        applyTDI(0);
        applyTDI(0);
        applyTDI(0);
        applyTDI(0);
        applyTDI(0);
        //applyTDI(0);

        // we are in Exit1-IR, go to Shift-DR
        applyTMS(2);
        applyTMS(2);
        applyTMS(2);
        applyTMS(0);
        applyTMS(0);

        // and read the IDCODES
        ui32by=33;
        program_array[0]=100;
        for(i=1;i<48;i++)
        {
            program_array[i] = scan_out_in(0);
            program_array2[i] = program_array[i];
        }
        write_function(program_array,16);

        //printf("IDCODE for device %d is %08X\n", i+1, JTAG_read(32));

        }
    }
    else if(state==1)
    {
		state=2;
		temp_in[0]=(uint8_t)(temp[0]);
		hex_INPIN = (temp[0]-1)/4;
		temp_in[1]=hex_INPIN+1;
		GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_5, 32);
    }

    else if(state==2)
    {
		state=3;
		temp_in[2]=(uint8_t)(temp[0]);
		hex_OUTPIN = (temp[0]-1)/4;
		temp_in[3]=hex_OUTPIN+1;
		GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_5, 32);


    }
    else if(state==3)
    {

        scanned_out_bits=0;
		while (hex_OUTPIN>=0 )
		{		binary[0]=0;binary[1]=0;binary[2]=0;binary[3]=0;
				t=3;
				while(t>=0)
				{
					binary[3-t] = scan_out_in(0);
					scanned_out_bits++;
					t--;

					if (scanned_out_bits==temp_in[2])
					{
						break;
					}

				}
				out_reg[hex_OUTPIN]=(binary[3]*8+binary[2]*4+binary[1]*2+binary[0])/32;
				hex_OUTPIN--;
			}

		while(hex_INPIN>=0)
		{
		    hextobin(temp[hex_INPIN]);
		    k=3;
				while(k>=0)
				{
					if (bin_eq[k] == '1')
					{
						applyTDI(1);
					}
					else
					{
						applyTDI(0);
					}
					scanned_in_bits++;
					if (scanned_in_bits == temp_in[0])
					{
						break;
					}
					k--;
				}
				hex_INPIN--;
		}
		scanned_out_bits=0;
		state=0;

    }

    //DEBUG_PRINT("Wrote %d bytes\n", ui32Count);


    // We processed as much data as we can directly from the receive buffer so
    // we need to return the number of bytes to allow the lower layer to
    // update its read pointer appropriately.
    //
    return(ui32Count);
}

//*****************************************************************************
//
// Handles bulk driver notifications related to the transmit channel (data to
// the USB host).
//
// \param pvCBData is the client-supplied callback pointer for this channel.
// \param ui32Event identifies the event we are being notified about.
// \param ui32MsgValue is an event-specific value.
// \param pvMsgData is an event-specific pointer.
//
// This function is called by the bulk driver to notify us of any events
// related to operation of the transmit data channel (the IN channel carrying
// data to the USB host).
//
// \return The return value is event-specific.
//
//*****************************************************************************
uint32_t
TxHandler(void *pvCBData, uint32_t ui32Event, uint32_t ui32MsgValue,
          void *pvMsgData)
{
    //
    // We are not required to do anything in response to any transmit event
    // in this example. All we do is update our transmit counter.
    //
    if(ui32Event == USB_EVENT_TX_COMPLETE)
    {
        g_ui32TxCount += ui32MsgValue;
    }

    //
    // Dump a debug message.
    //
    DEBUG_PRINT("TX complete %d\n", ui32MsgValue);

    return(0);
}

//*****************************************************************************
//
// Handles bulk driver notifications related to the receive channel (data from
// the USB host).
//
// \param pvCBData is the client-supplied callback pointer for this channel.
// \param ui32Event identifies the event we are being notified about.
// \param ui32MsgValue is an event-specific value.
// \param pvMsgData is an event-specific pointer.
//
// This function is called by the bulk driver to notify us of any events
// related to operation of the receive data channel (the OUT channel carrying
// data from the USB host).
//
// \return The return value is event-specific.
//
//*****************************************************************************
uint32_t
RxHandler(void *pvCBData, uint32_t ui32Event,
               uint32_t ui32MsgValue, void *pvMsgData)
{
    //
    // Which event are we being sent?
    //
    switch(ui32Event)
    {
        //
        // We are connected to a host and communication is now possible.
        //
        case USB_EVENT_CONNECTED:
        {
            g_bUSBConfigured = true;
            //UARTprintf("Host connected.\n");

            //
            // Flush our buffers.
            //
            USBBufferFlush(&g_sTxBuffer);
            USBBufferFlush(&g_sRxBuffer);

            break;
        }

        //
        // The host has disconnected.
        //
        case USB_EVENT_DISCONNECTED:
        {
            g_bUSBConfigured = false;
            //UARTprintf("Host disconnected.\n");
            break;
        }

        //
        // A new packet has been received.
        //
        case USB_EVENT_RX_AVAILABLE:
        {
            tUSBDBulkDevice *psDevice;

            //
            // Get a pointer to our instance data from the callback data
            // parameter.
            //
            psDevice = (tUSBDBulkDevice *)pvCBData;

            //
            // Read the new packet and echo it back to the host.
            //
            return(EchoNewDataToHost(psDevice, pvMsgData, ui32MsgValue));
        }

        //
        // Ignore SUSPEND and RESUME for now.
        //
        case USB_EVENT_SUSPEND:
        case USB_EVENT_RESUME:
        {
            break;
        }

        //
        // Ignore all other events and return 0.
        //
        default:
        {
            break;
        }
    }

    return(0);
}

//*****************************************************************************
//
// Configure the UART and its pins.  This must be called before UARTprintf().
//
//*****************************************************************************
/*
void
ConfigureUART(void)
{
    //
    // Enable the GPIO Peripheral used by the UART.
    //
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    //
    // Enable UART0
    //
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    //
    // Configure GPIO Pins for UART mode.
    //
    ROM_GPIOPinConfigure(GPIO_PA0_U0RX);
    ROM_GPIOPinConfigure(GPIO_PA1_U0TX);
    ROM_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE, GPIO_PIN_5);
    //
    // Use the internal 16MHz oscillator as the UART clock source.
    //
    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);

    //
    // Initialize the UART for console I/O.
    //
    UARTStdioConfig(0, 115200, 16000000);
}
*/
//*****************************************************************************
//
// This is the main application entry function.
//
//*****************************************************************************
int
main(void)
{
    volatile uint32_t ui32Loop;
    uint32_t ui32TxCount;
    uint32_t ui32RxCount;

    //
    // Enable lazy stacking for interrupt handlers.  This allows floating-point
    // instructions to be used within interrupt handlers, but at the expense of
    // extra stack usage.
    //
    ROM_FPULazyStackingEnable();

    //
    // Set the clocking to run from the PLL at 50MHz
    //
    ROM_SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN |
                       SYSCTL_XTAL_16MHZ);

    //
    // Enable the GPIO port that is used for the on-board LED.
    //
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

    //
    // Enable the GPIO pins for the LED (PF2 & PF3).
    //
    ROM_GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_3 | GPIO_PIN_2|GPIO_PIN_1);
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3 | GPIO_PIN_2|GPIO_PIN_1, 8);
    //
    // Open UART0 and show the application name on the UART.
    //
    //ConfigureUART();
    Sys_config_cpld();
    //UARTprintf("\033[2JTiva C Series USB bulk device example\n");
    //UARTprintf("---------------------------------\n\n");

    //
    // Not configured initially.
    //
    g_bUSBConfigured = false;

    //
    // Enable the GPIO peripheral used for USB, and configure the USB
    // pins.
    //
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    ROM_GPIOPinTypeUSBAnalog(GPIO_PORTD_BASE, GPIO_PIN_4 | GPIO_PIN_5);

    //
    // Enable the system tick.
    //
    ROM_SysTickPeriodSet(ROM_SysCtlClockGet() / SYSTICKS_PER_SECOND);
    ROM_SysTickIntEnable();
    ROM_SysTickEnable();

    //
    // Tell the user what we are up to.
    //
    //UARTprintf("Configuring USB\n");

    //
    // Initialize the transmit and receive buffers.
    //
    USBBufferInit(&g_sTxBuffer);
    USBBufferInit(&g_sRxBuffer);

    //
    // Set the USB stack mode to Device mode with VBUS monitoring.
    //
    USBStackModeSet(0, eUSBModeForceDevice, 0);

    //
    // Pass our device information to the USB library and place the device
    // on the bus.
    //
    USBDBulkInit(0, &g_sBulkDevice);

    //
    // Wait for initial configuration to complete.
    //
    //UARTprintf("Waiting for host...\n");

    //
    // Clear our local byte counters.
    //
    ui32RxCount = 0;
    ui32TxCount = 0;

    //
    // Main application loop.
    //
    while(1)
    {
        //
        // See if any data has been transferred.
        //
        if((ui32TxCount != g_ui32TxCount) || (ui32RxCount != g_ui32RxCount))
        {
            //
            // Has there been any transmit traffic since we last checked?
            //
            if(ui32TxCount != g_ui32TxCount)
            {
                //
                // Turn on the Green LED.
                //
                //GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, GPIO_PIN_3);

                //
                // Delay for a bit.
                //
                //for(ui32Loop = 0; ui32Loop < 150000; ui32Loop++)
                //{
                //}

                //
                // Turn off the Green LED.
                //
                //GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0);

                //
                // Take a snapshot of the latest transmit count.
                //
                ui32TxCount = g_ui32TxCount;
            }

            //
            // Has there been any receive traffic since we last checked?
            //
            if(ui32RxCount != g_ui32RxCount)
            {
                //
                // Turn on the Blue LED.
                //
                //GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, GPIO_PIN_2);

                //
                // Delay for a bit.
                //
                //for(ui32Loop = 0; ui32Loop < 150000; ui32Loop++)
                //{
                //}

                //
                // Turn off the Blue LED.
                //
                //GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0);

                //
                // Take a snapshot of the latest receive count.
                //
                ui32RxCount = g_ui32RxCount;
            }

            //
            // Update the display of bytes transferred.
            //
           // UARTprintf("\rTx: %d  Rx: %d", ui32TxCount, ui32RxCount);
        }
    }
}

