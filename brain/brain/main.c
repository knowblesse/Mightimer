/*
 * main.c
 *
 * Created: 10/31/2022 3:08:52 PM
 *  Author: Knowblesse
 */ 
#define F_CPU 32768UL
#include <xc.h>
#include <util/delay.h>
//#include "display.h"
//#include "heximage.h"
#define USART0_BAUD_RATE(BAUD_RATE) ((float)(F_CPU * 64 / (16 * (float)BAUD_RATE)) + 0.5)

int main(void)
{
	/*
	PORTA.DIRSET = 0b01000001; //pin PA0 and PA6 to OUTPUT
	PORTA.DIRCLR = 0b00000010; //pin PA1 to INPUT
	int data = 0;
	USART0.BAUD = USART0_BAUD_RATE(1200); // set BAUD RATE
	USART0.CTRLC = 0b00000011; // set "send 8bits per frame"
	USART0.CTRLB = 0b11000000; // enable Tx and Rx
	
    while(1)
    {
        //TODO:: Please write your application code 
		PORTA.OUTSET = 0x40; // pinPA0 HIGH
		_delay_ms(1000);
		USART0.TXDATAL = (uint16_t)data; 
		PORTA.OUTCLR = 0x40; // pinPA0 LOW
		_delay_ms(4000);
		data++;
    }
	*/
	//Digital Output Pin Setup
	PORTA.DIRSET |= PIN7_bm; // Chip Select Pin
	PORTC.DIRSET |= (PIN1_bm & PIN2_bm); // LCD Reset, A0 Pin Output
	
	//SPI Setup
	SPI0.CTRLA |= SPI_MASTER_bm;
	SPI0.CTRLA |= SPI_ENABLE_bm;
	
	while(1)
	{
		
		
		
	}
}