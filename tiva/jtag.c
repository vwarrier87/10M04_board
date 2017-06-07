#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "jtag.h"


uint32_t jeetaaaggg = 0;

uint8_t JTAG_clock(uint32_t val)
{
	GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_4, val);
	GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_5,0);
	//Delay of 10us
	SysCtlDelay(SysCtlClockGet()/(3*100000));
	GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_5,32);
	//Delay of 10us
	SysCtlDelay(SysCtlClockGet()/(3*100000));
	if(GPIOPinRead(GPIO_PORTB_BASE, GPIO_PIN_5) == GPIO_PIN_5)
		return 1;
	else
		return 0;
}

uint8_t JTAG_read()
{
    GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_5,0);
    //Delay of 10us
    SysCtlDelay(SysCtlClockGet()/(3*100000));
    GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_5,32);
    //Delay of 10us
    SysCtlDelay(SysCtlClockGet()/(3*100000));
    if(GPIOPinRead(GPIO_PORTB_BASE, GPIO_PIN_5) == GPIO_PIN_5)
        return 1;
    else
        return 0;
}

void jtag_ir_write(uint8_t ir_len, uint32_t instruction)
{
    volatile uint8_t i=0,j=0;
    volatile uint32_t mask=0, instr = 0, k=0;

    instr = instruction;

    for(i=0;i<ir_len;i++)
    {
        mask = 1<<i;
        k = instr & mask ;
        if(i == ir_len-1)
        {
            if(k == mask)
            {
                JTAG_clock(TDI | TMS);
            }
            else
            {
                JTAG_clock(0 | TMS);
            }
        }
        else
        {
            if(k == mask)
            {
                JTAG_clock(TDI);
            }
            else
            {
                JTAG_clock(0);
            }
        }
    }

}


void jtag_set_state(uint8_t state)
{
    volatile uint8_t i=0;
    for(i=0; i<5; i++)
        JTAG_clock(TMS);
    switch(state)
    {
    case 0:
        //already in reset state
        break;
    case 1:
        //idle state
        JTAG_clock(0);
        break;
    case 2:
        //drselect state
        JTAG_clock(0);
        JTAG_clock(TMS);
        break;
    case 3:
        //drcapture state
        JTAG_clock(0);
        JTAG_clock(TMS);
        JTAG_clock(0);
        break;
    case 4:
        //drshift state
        JTAG_clock(0);
        JTAG_clock(TMS);
        JTAG_clock(0);
        JTAG_clock(0);
        break;
    case 5:
        //drexit1 state
        JTAG_clock(0);
        JTAG_clock(TMS);
        JTAG_clock(0);
        JTAG_clock(0);
        JTAG_clock(TMS);
        break;
    case 6:
        //drpause state
        JTAG_clock(0);
        JTAG_clock(TMS);
        JTAG_clock(0);
        JTAG_clock(0);
        JTAG_clock(TMS);
        JTAG_clock(0);
        break;
    case 7:
        //drexit2 state
        JTAG_clock(0);
        JTAG_clock(TMS);
        JTAG_clock(0);
        JTAG_clock(0);
        JTAG_clock(TMS);
        JTAG_clock(0);
        JTAG_clock(TMS);
        break;
    case 8:
        //drupdate state
        JTAG_clock(0);
        JTAG_clock(TMS);
        JTAG_clock(0);
        JTAG_clock(0);
        JTAG_clock(TMS);
        JTAG_clock(0);
        JTAG_clock(TMS);
        JTAG_clock(TMS);
        break;
    case 9:
        //irselect state
        JTAG_clock(0);
        JTAG_clock(TMS);
        JTAG_clock(TMS);
        break;
    case 10:
        //ircapture state
        JTAG_clock(0);
        JTAG_clock(TMS);
        JTAG_clock(TMS);
        JTAG_clock(0);
        break;
    case 11:
        //irshift state
        JTAG_clock(0);
        JTAG_clock(TMS);
        JTAG_clock(TMS);
        JTAG_clock(0);
        JTAG_clock(0);
        break;
    case 12:
        //irexit1 state
        JTAG_clock(0);
        JTAG_clock(TMS);
        JTAG_clock(TMS);
        JTAG_clock(0);
        JTAG_clock(0);
        JTAG_clock(TMS);
        break;
    case 13:
        //irpause state
        JTAG_clock(0);
        JTAG_clock(TMS);
        JTAG_clock(TMS);
        JTAG_clock(0);
        JTAG_clock(0);
        JTAG_clock(TMS);
        JTAG_clock(0);
        break;
    case 14:
        //irexit2 state
        JTAG_clock(0);
        JTAG_clock(TMS);
        JTAG_clock(TMS);
        JTAG_clock(0);
        JTAG_clock(0);
        JTAG_clock(TMS);
        JTAG_clock(0);
        JTAG_clock(TMS);
        break;
    case 15:
        //irupdate state
        JTAG_clock(0);
        JTAG_clock(TMS);
        JTAG_clock(TMS);
        JTAG_clock(0);
        JTAG_clock(0);
        JTAG_clock(TMS);
        JTAG_clock(0);
        JTAG_clock(TMS);
        JTAG_clock(TMS);
        break;
    default:
        break;

    }
}

void jtag_change_state(uint8_t curr_state, uint8_t next_state)
{
    volatile uint8_t i=0;
    switch(curr_state)
    {
    case 0:
        switch(next_state)
        {
        case 0:
            //already in reset state
            break;
        case 1:
            //idle state
            JTAG_clock(0);
            break;
        case 2:
            //drselect state
            JTAG_clock(0);
            JTAG_clock(TMS);
            break;
        case 3:
            //drcapture state
            JTAG_clock(0);
            JTAG_clock(TMS);
            JTAG_clock(0);
            break;
        case 4:
            //drshift state
            JTAG_clock(0);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(0);
            break;
        case 5:
            //drexit1 state
            JTAG_clock(0);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            break;
        case 6:
            //drpause state
            JTAG_clock(0);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            JTAG_clock(0);
            break;
        case 7:
            //drexit2 state
            JTAG_clock(0);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            break;
        case 8:
            //drupdate state
            JTAG_clock(0);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            break;
        case 9:
            //irselect state
            JTAG_clock(0);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            break;
        case 10:
            //ircapture state
            JTAG_clock(0);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            break;
        case 11:
            //irshift state
            JTAG_clock(0);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(0);
            break;
        case 12:
            //irexit1 state
            JTAG_clock(0);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            break;
        case 13:
            //irpause state
            JTAG_clock(0);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            JTAG_clock(0);
            break;
        case 14:
            //irexit2 state
            JTAG_clock(0);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            break;
        case 15:
            //irupdate state
            JTAG_clock(0);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            break;
        default:
            break;
        }
        break;
    case 1:
		//idle state
        switch(next_state)
        {
        case 0:
			//reset state
			for(i=0; i<5; i++)
				JTAG_clock(TMS);
            break;
        case 1:
            //already in idle state
            break;
        case 2:
            //drselect state
            JTAG_clock(TMS);
            break;
        case 3:
            //drcapture state
            JTAG_clock(TMS);
            JTAG_clock(0);
            break;
        case 4:
            //drshift state
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(0);
            break;
        case 5:
            //drexit1 state
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            break;
        case 6:
            //drpause state
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            JTAG_clock(0);
            break;
        case 7:
            //drexit2 state
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            break;
        case 8:
            //drupdate state
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            break;
        case 9:
            //irselect state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            break;
        case 10:
            //ircapture state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            break;
        case 11:
            //irshift state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(0);
            break;
        case 12:
            //irexit1 state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            break;
        case 13:
            //irpause state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            JTAG_clock(0);
            break;
        case 14:
            //irexit2 state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            break;
        case 15:
            //irupdate state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            break;
        default:
            break;
        }
        break;
    case 2:
        switch(next_state)
        {
        case 0:
			//reset state
			for(i=0; i<5; i++)
				JTAG_clock(TMS);
            break;
        case 1:
            //idle state
            for(i=0; i<5; i++)
				JTAG_clock(TMS);
			JTAG_clock(0);
            break;
        case 2:
            //already in drselect state
            break;
        case 3:
            //drcapture state
            JTAG_clock(0);
            break;
        case 4:
            //drshift state
            JTAG_clock(0);
            JTAG_clock(0);
            break;
        case 5:
            //drexit1 state
            JTAG_clock(0);
            JTAG_clock(TMS);
            break;
        case 6:
            //drpause state
            JTAG_clock(0);
            JTAG_clock(TMS);
            JTAG_clock(0);
            break;
        case 7:
            //drexit2 state
            JTAG_clock(0);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            break;
        case 8:
            //drupdate state
            JTAG_clock(0);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            break;
        case 9:
            //irselect state
            JTAG_clock(TMS);
            break;
        case 10:
            //ircapture state
            JTAG_clock(TMS);
            JTAG_clock(0);
            break;
        case 11:
            //irshift state
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(0);
            break;
        case 12:
            //irexit1 state
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            break;
        case 13:
            //irpause state
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            JTAG_clock(0);
            break;
        case 14:
            //irexit2 state
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            break;
        case 15:
            //irupdate state
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            break;
        default:
            break;
        }
        break;
    case 3:
		//drcapture state
        switch(next_state)
        {
        case 0:
			//reset state
			for(i=0; i<5; i++)
				JTAG_clock(TMS);
            break;
        case 1:
            //idle state
            for(i=0; i<5; i++)
				JTAG_clock(TMS);
			JTAG_clock(0);
            break;
        case 2:
            //drselect state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            break;
        case 3:
            //already in drcapture state
            break;
        case 4:
            //drshift state
            JTAG_clock(0);
            break;
        case 5:
            //drexit1 state
            JTAG_clock(TMS);
            break;	
        case 6:
            //drpause state
            JTAG_clock(TMS);
            JTAG_clock(0);
            break;
        case 7:
            //drexit2 state
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            break;
        case 8:
            //drupdate state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            break;
        case 9:
            //irselect state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            break;
        case 10:
            //ircapture state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            break;
        case 11:
            //irshift state
            JTAG_clock(0);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(0);
            break;
        case 12:
            //irexit1 state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            break;
        case 13:
            //irpause state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            JTAG_clock(0);
            break;
        case 14:
            //irexit2 state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            break;
        case 15:
            //irupdate state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            break;
        default:
            break;
        }
        break;
    case 4:
        //drshift state
        switch(next_state)
        {
        case 0:
			//reset state
			for(i=0; i<5; i++)
				JTAG_clock(TMS);
            break;
        case 1:
            //idle state
            for(i=0; i<5; i++)
				JTAG_clock(TMS);
			JTAG_clock(0);
            break;
        case 2:
            //drselect state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            break;
        case 3:
            //drcapture state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            break;
        case 4:
            //already in drshift state
            break;
        case 5:
            //drexit1 state
            JTAG_clock(TMS);
            break;	
        case 6:
            //drpause state
            JTAG_clock(TMS);
            JTAG_clock(0);
            break;
        case 7:
            //drexit2 state
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            break;
        case 8:
            //drupdate state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            break;
        case 9:
            //irselect state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            break;
        case 10:
            //ircapture state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            break;
        case 11:
            //irshift state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(0);
            break;
        case 12:
            //irexit1 state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            break;
        case 13:
            //irpause state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            JTAG_clock(0);
            break;
        case 14:
            //irexit2 state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            break;
        case 15:
            //irupdate state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            break;
        default:
            break;
        }
        break;
    case 5:
        //drexit1 state
        switch(next_state)
        {
        case 0:
			//reset state
			for(i=0; i<5; i++)
				JTAG_clock(TMS);
            break;
        case 1:
            //idle state
            JTAG_clock(TMS);
			JTAG_clock(0);
            break;
        case 2:
            //drselect state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            break;
        case 3:
            //drcapture state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            break;
        case 4:
            //drshift state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(0);
            break;
        case 5:
            //already drexit1 state
            break;	
        case 6:
            //drpause state
            JTAG_clock(0);
            break;
        case 7:
            //drexit2 state
            JTAG_clock(0);
            JTAG_clock(TMS);
            break;
        case 8:
            //drupdate state
            JTAG_clock(TMS);
            break;
        case 9:
            //irselect state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            break;
        case 10:
            //ircapture state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            break;
        case 11:
            //irshift state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(0);
            break;
        case 12:
            //irexit1 state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            break;
        case 13:
            //irpause state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            JTAG_clock(0);
            break;
        case 14:
            //irexit2 state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            break;
        case 15:
            //irupdate state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            break;
        default:
            break;
        }
        break;
    case 6:
        //drpause state
        switch(next_state)
        {
        case 0:
			//reset state
			for(i=0; i<5; i++)
				JTAG_clock(TMS);
            break;
        case 1:
            //idle state
            JTAG_clock(TMS);
			JTAG_clock(TMS);
			JTAG_clock(0);
            break;
        case 2:
            //drselect state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            break;
        case 3:
            //drcapture state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            break;
        case 4:
            //drshift state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(0);
            break;
        case 5:
            //drexit1 state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            break;	
        case 6:
            //already in drpause state
            break;
        case 7:
            //drexit2 state
            JTAG_clock(TMS);
            break;
        case 8:
            //drupdate state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            break;
        case 9:
            //irselect state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            break;
        case 10:
            //ircapture state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            break;
        case 11:
            //irshift state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(0);
            break;
        case 12:
            //irexit1 state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            break;
        case 13:
            //irpause state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            JTAG_clock(0);
            break;
        case 14:
            //irexit2 state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            break;
        case 15:
            //irupdate state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            break;
        default:
            break;
        }
        break;
    case 7:
        //drexit2 state
        switch(next_state)
        {
        case 0:
			//reset state
			for(i=0; i<5; i++)
				JTAG_clock(TMS);
            break;
        case 1:
            //idle state
            JTAG_clock(TMS);
			JTAG_clock(0);
            break;
        case 2:
            //drselect state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            break;
        case 3:
            //drcapture state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            break;
        case 4:
            //drshift state
            JTAG_clock(0);
            break;
        case 5:
            //drexit1 state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            break;	
        case 6:
            //drpause state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
			JTAG_clock(0);
            break;
        case 7:
            //already in drexit2 state
            break;
        case 8:
            //drupdate state
            JTAG_clock(TMS);
            break;
        case 9:
            //irselect state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            break;
        case 10:
            //ircapture state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            break;
        case 11:
            //irshift state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(0);
            break;
        case 12:
            //irexit1 state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            break;
        case 13:
            //irpause state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            JTAG_clock(0);
            break;
        case 14:
            //irexit2 state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            break;
        case 15:
            //irupdate state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            break;
        default:
            break;
        }
        break;
    case 8:
        //drupdate state
        switch(next_state)
        {
        case 0:
			//reset state
			for(i=0; i<5; i++)
				JTAG_clock(TMS);
            break;
        case 1:
            //idle state
			JTAG_clock(0);
            break;
        case 2:
            //drselect state
            JTAG_clock(TMS);
            break;
        case 3:
            //drcapture state
            JTAG_clock(TMS);
            JTAG_clock(0);
            break;
        case 4:
            //drshift state
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(0);
            break;
        case 5:
            //drexit1 state
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            break;	
        case 6:
            //drpause state
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            JTAG_clock(0);
            break;
        case 7:
            //drexit2 state
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            break;
        case 8:
            //already in drupdate state
            break;
        case 9:
            //irselect state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            break;
        case 10:
            //ircapture state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            break;
        case 11:
            //irshift state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(0);
            break;
        case 12:
            //irexit1 state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            break;
        case 13:
            //irpause state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            JTAG_clock(0);
            break;
        case 14:
            //irexit2 state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            break;
        case 15:
            //irupdate state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            break;
        default:
            break;
        }
        break;
    case 9:
        //irselect state
        switch(next_state)
        {
        case 0:
			//reset state
			for(i=0; i<5; i++)
				JTAG_clock(TMS);
            break;
        case 1:
            //idle state
            JTAG_clock(0);
			JTAG_clock(TMS);
			JTAG_clock(TMS);
			JTAG_clock(0);
            break;
        case 2:
            //drselect state
            JTAG_clock(0);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            break;
        case 3:
            //drcapture state
            JTAG_clock(0);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            break;
        case 4:
            //drshift state
            JTAG_clock(0);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(0);
            break;
        case 5:
            //drexit1 state
            JTAG_clock(0);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            break;	
        case 6:
            //drpause state
            JTAG_clock(0);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            JTAG_clock(0);
            break;
        case 7:
            //drexit2 state
            JTAG_clock(0);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            break;
        case 8:
            //drupdate state
            JTAG_clock(0);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            break;
        case 9:
            //already in irselect state
            break;
        case 10:
            //ircapture state
            JTAG_clock(0);
            break;
        case 11:
            //irshift state
            JTAG_clock(0);
            JTAG_clock(0);
            break;
        case 12:
            //irexit1 state
            JTAG_clock(0);
            JTAG_clock(TMS);
            break;
        case 13:
            //irpause state
            JTAG_clock(0);
            JTAG_clock(TMS);
            JTAG_clock(0);
            break;
        case 14:
            //irexit2 state
            JTAG_clock(0);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            break;
        case 15:
            //irupdate state
            JTAG_clock(0);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            break;
        default:
            break;
        }
        break;
    case 10:
        //ircapture state
        switch(next_state)
        {
        case 0:
			//reset state
			for(i=0; i<5; i++)
				JTAG_clock(TMS);
            break;
        case 1:
            //idle state
			JTAG_clock(TMS);
			JTAG_clock(TMS);
			JTAG_clock(0);
            break;
        case 2:
            //drselect state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            break;
        case 3:
            //drcapture state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            break;
        case 4:
            //drshift state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(0);
            break;
        case 5:
            //drexit1 state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            break;	
        case 6:
            //drpause state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            JTAG_clock(0);
            break;
        case 7:
            //drexit2 state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            break;
        case 8:
            //drupdate state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            break;
        case 9:
            //irselect state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            break;
        case 10:
            //already in ircapture state
            break;
        case 11:
            //irshift state
            JTAG_clock(0);
            break;
        case 12:
            //irexit1 state
            JTAG_clock(TMS);
            break;
        case 13:
            //irpause state
            JTAG_clock(TMS);
            JTAG_clock(0);
            break;
        case 14:
            //irexit2 state
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            break;
        case 15:
            //irupdate state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            break;
        default:
            break;
        }
        break;
    case 11:
        //irshift state
        switch(next_state)
        {
        case 0:
			//reset state
			for(i=0; i<5; i++)
				JTAG_clock(TMS);
            break;
        case 1:
			JTAG_clock(TMS);
			JTAG_clock(TMS);	
			JTAG_clock(0);
            break;
        case 2:
            //drselect state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            break;
        case 3:
            //drcapture state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            break;
        case 4:
            //drshift state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(0);
            break;
        case 5:
            //drexit1 state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            break;	
        case 6:
            //drpause state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            JTAG_clock(0);
            break;
        case 7:
            //drexit2 state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            break;
        case 8:
            //drupdate state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            break;
        case 9:
            //irselect state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            break;
        case 10:
            //ircapture state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            break;
        case 11:
            //already in irshift state
            break;
        case 12:
            //irexit1 state
            JTAG_clock(TMS);
            break;
        case 13:
            //irpause state
            JTAG_clock(TMS);
            JTAG_clock(0);
            break;
        case 14:
            //irexit2 state
            JTAG_clock(0);
            break;
        case 15:
            //irupdate state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            break;
        default:
            break;
        }
        break;
    case 12:
        //irexit1 state
        switch(next_state)
        {
        case 0:
			//reset state
			for(i=0; i<5; i++)
				JTAG_clock(TMS);
            break;
        case 1:
            //idle state
			JTAG_clock(TMS);
			JTAG_clock(0);
            break;
        case 2:
            //drselect state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            break;
        case 3:
            //drcapture state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            break;
        case 4:
            //drshift state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(0);
            break;
        case 5:
            //drexit1 state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            break;	
        case 6:
            //drpause state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            JTAG_clock(0);
            break;
        case 7:
            //drexit2 state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            break;
        case 8:
            //drupdate state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            break;
        case 9:
            //irselect state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            break;
        case 10:
            //ircapture state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            break;
        case 11:
            //irshift state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(0);
            break;
        case 12:
            //already in irexit1 state
            break;
        case 13:
            //irpause state
            JTAG_clock(0);
            break;
        case 14:
            //irexit2 state
            JTAG_clock(0);
            JTAG_clock(TMS);
            break;
        case 15:
            //irupdate state
            JTAG_clock(TMS);
            break;
        default:
            break;
        }
        break;
    case 13:
        //irpause state
        switch(next_state)
        {
        case 0:
			//reset state
			for(i=0; i<5; i++)
				JTAG_clock(TMS);
            break;
        case 1:
            //idle state
            JTAG_clock(TMS);
			JTAG_clock(TMS);
			JTAG_clock(0);
            break;
        case 2:
            //drselect state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            break;
        case 3:
            //drcapture state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            break;
        case 4:
            //drshift state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(0);
            break;
        case 5:
            //drexit1 state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            break;	
        case 6:
            //drpause state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            JTAG_clock(0);
            break;
        case 7:
            //drexit2 state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            break;
        case 8:
            //drupdate state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            break;
        case 9:
            //irselect state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            break;
        case 10:
            //ircapture state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            break;
        case 11:
            //irshift state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(0);
            break;
        case 12:
            //irexit1 state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            break;
        case 13:
            //already in irpause state
            break;
        case 14:
            //irexit2 state
            JTAG_clock(TMS);
            break;
        case 15:
            //irupdate state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            break;
        default:
            break;
        }
        break;
    case 14:
        //irexit2 state
        switch(next_state)
        {
        case 0:
			//reset state
			for(i=0; i<5; i++)
				JTAG_clock(TMS);
            break;
        case 1:
            //idle state
			JTAG_clock(TMS);
			JTAG_clock(0);
            break;
        case 2:
            //drselect state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            break;
        case 3:
            //drcapture state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            break;
        case 4:
            //drshift state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(0);
            break;
        case 5:
            //drexit1 state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            break;	
        case 6:
            //drpause state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            JTAG_clock(0);
            break;
        case 7:
            //drexit2 state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            break;
        case 8:
            //drupdate state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            break;
        case 9:
            //irselect state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            break;
        case 10:
            //ircapture state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            break;
        case 11:
            //irshift state
            JTAG_clock(0);
            break;
        case 12:
            //irexit1 state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            break;
        case 13:
            //irpause state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            JTAG_clock(0);
            break;
        case 14:
            //already in irexit2 state
            break;
        case 15:
            //irupdate state
            JTAG_clock(TMS);
            break;
        default:
            break;
        }
        break;
    case 15:
        //irupdate state
        switch(next_state)
        {
        case 0:
			//reset state
			for(i=0; i<5; i++)
				JTAG_clock(TMS);
            break;
        case 1:
            //idle state
			JTAG_clock(0);
            break;
        case 2:
            //drselect state
            JTAG_clock(TMS);
            break;
        case 3:
            //drcapture state
            JTAG_clock(TMS);
            JTAG_clock(0);
            break;
        case 4:
            //drshift state
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(0);
            break;
        case 5:
            //drexit1 state
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            break;	
        case 6:
            //drpause state
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            JTAG_clock(0);
            break;
        case 7:
            //drexit2 state
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            break;
        case 8:
            //drupdate state
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            break;
        case 9:
            //irselect state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            break;
        case 10:
            //ircapture state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            break;
        case 11:
            //irshift state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(0);
            break;
        case 12:
            //irexit1 state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            break;
        case 13:
            //irpause state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            JTAG_clock(0);
            break;
        case 14:
            //irexit2 state
            JTAG_clock(TMS);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            JTAG_clock(0);
            JTAG_clock(TMS);
            break;
        case 15:
            //already in irupdate state
            break;
        default:
            break;
        }
        break;
    default:
        break;

    }
}

