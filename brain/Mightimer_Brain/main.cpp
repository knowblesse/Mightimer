/*
 * Mightimer_Brain.cpp
 *
 * Created: 2023-02-22 오후 5:41:37
 * Author : Knowblesse
 */ 

#define F_CPU 32768UL
//#define F_CPU 32000000UL
#include <avr/io.h>
#include <util/delay.h>
#include "display.h"
#include "heximage.h"
#define USART0_BAUD_RATE(BAUD_RATE) ((float)(F_CPU * 64 / (16 * (float)BAUD_RATE)) + 0.5)

int main(void)
{	
	PORTA.DIRSET = PIN0_bm | PIN3_bm; //pin PA0 (Tx), PA3(LED) to OUTPUT
	PORTA.DIRCLR = PIN1_bm; //pin PA1(Rx) to INPUT
	int data = 0;
	USART0.BAUD = USART0_BAUD_RATE(1200); // set BAUD RATE
	USART0.CTRLC = 0b00000011; // set "send 8bits per frame"
	USART0.CTRLB = 0b11000000; // enable Tx and Rx

	_delay_ms(100);
	PORTC.OUTSET = PIN1_bm;
	
	_delay_ms(2000);
	
	SPI_Display spiDisplay = SPI_Display();
	spiDisplay.init_LCD();
	spiDisplay.ClearLCD();
	spiDisplay.DispPic(background);
	
	
    while(1)
    {
		for(int i = 0; i < 8; i++)
		{
			spiDisplay.setHour(i);
			_delay_ms(1000);
			spiDisplay.ClearLCD();
		}
		//USART0.TXDATAL = (uint16_t)data; 
		//PORTA.OUTCLR = PIN7_bm;
		//SPI0.DATA = 0xF6;
		//while(!(SPI0.INTFLAGS & SPI_IF_bm));
		//PORTA.OUTSET = PIN7_bm;
		//_delay_ms(1000);
		//data++;
    }
}

