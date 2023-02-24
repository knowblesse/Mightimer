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

int currSecond = -1;
int currMinute = -1;
int currHour = -1;

void setTime(SPI_Display *spiDisplay, long long timeInSec)
{
	int hour = (int)((double)timeInSec / 3600.0);
	int minute = (int)((double)(timeInSec - 3600*hour) / 60.0);
	int second = timeInSec % 60;
	
	if(currSecond != second) spiDisplay->setSecond(second);
	if(currMinute != minute) spiDisplay->setMinute(minute);
	if(currHour != hour) spiDisplay->setHour(hour);
}

int main(void)
{	
	//Setup External 32kHz Clock
	CLKCTRL.MCLKCTRLA |= CLKCTRL_CLKSEL_XOSC32K_gc;
	
	
	PORTA.DIRSET = PIN0_bm | PIN3_bm; //pin PA0 (Tx), PA3(LED) to OUTPUT
	PORTA.DIRCLR = PIN1_bm; //pin PA1(Rx) to INPUT
	PORTC.DIRSET = PIN3_bm;
	
	USART0.BAUD = USART0_BAUD_RATE(1200); // set BAUD RATE
	USART0.CTRLC = 0b00000011; // set "send 8bits per frame"
	USART0.CTRLB = 0b11000000; // enable Tx and Rx
	
	RTC.CLKSEL |= RTC_CLKSEL_OSC32K_gc;
	RTC.CTRLA |= RTC_RTCEN_bm;
	while((RTC.PITSTATUS & RTC_CTRLABUSY_bm));
	PORTC.OUTCLR = PIN3_bm;
	
	RTC.PITCTRLA |= RTC_PERIOD_CYC32768_gc;
	RTC.PITINTCTRL = RTC_PI_bm;
	RTC.PITCTRLA |= RTC_PITEN_bm;
	RTC.CALIB = 0x10;
	
	// SPI Display Setup
	SPI_Display spiDisplay = SPI_Display();
	spiDisplay.init_LCD();
	spiDisplay.ClearLCD();
	spiDisplay.DispPic(background);
	setTime(&spiDisplay, 0);
	
	int currTimeInSec = 0;
    while(1)
    {
		if(RTC.PITINTFLAGS & RTC_PI_bm)
		{
			RTC.PITINTFLAGS = 1;
			currTimeInSec++;
			setTime(&spiDisplay, currTimeInSec);
		}
		//for(int i = 0; i < 8; i++)
		//{
			//spiDisplay.setHour(i);
			//_delay_ms(1000);
			//spiDisplay.ClearLCD();
		//}
		//USART0.TXDATAL = (uint16_t)data; 
		//PORTA.OUTCLR = PIN7_bm;
		//SPI0.DATA = 0xF6;
		//while(!(SPI0.INTFLAGS & SPI_IF_bm));
		//PORTA.OUTSET = PIN7_bm;
		//_delay_ms(1000);
		//data++;
    }
}

