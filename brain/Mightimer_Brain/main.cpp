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
// TODO: still very wrong timing. (maybe PIT is not a good choice?)
int main(void)
{	
	
	//TODO: maybe disabling and reenabling is not necessary
	// Disable External 32kHz oscillator
	uint8_t temp = CLKCTRL.XOSC32KCTRLA;
	temp &= ~CLKCTRL_ENABLE_bm;
	CPU_CCP = CCP_IOREG_gc; // enable changing protected bit
	CLKCTRL.XOSC32KCTRLA = temp;
	
	// Check Disabled
	while ((CLKCTRL.MCLKSTATUS & CLKCTRL_XOSC32KS_bm));

	// Set XOSC32K
	temp = CLKCTRL.XOSC32KCTRLA;
	temp &= ~CLKCTRL_SEL_bm; // select i'm using two pins for oscillator
	temp |= CLKCTRL_RUNSTBY_bm; // always running. maybe not necessary
	CPU_CCP = CCP_IOREG_gc;
	CLKCTRL.XOSC32KCTRLA = temp;
	
	// Enable External 32kHz oscillator
	temp = CLKCTRL.XOSC32KCTRLA;
	temp |= CLKCTRL_ENABLE_bm;
	CPU_CCP = CCP_IOREG_gc;
	CLKCTRL.XOSC32KCTRLA = temp;
	
	// Check enabled
	while (!(CLKCTRL.MCLKSTATUS & CLKCTRL_XOSC32KS_bm));	
	
	// Select main source as XOSC32K
	temp = CLKCTRL.MCLKCTRLA;
	temp = CLKCTRL_CLKSEL_XOSC32K_gc;
	CPU_CCP = CCP_IOREG_gc;
	CLKCTRL.MCLKCTRLA = temp;
	
	// Check if main clock is synced
	while (CLKCTRL.MCLKSTATUS & CLKCTRL_SOSC_bm);
	
	// Setup Ports	
	PORTA.DIRSET = PIN0_bm | PIN3_bm; //pin PA0 (Tx), PA3(LED) to OUTPUT
	PORTA.DIRCLR = PIN1_bm; //pin PA1(Rx) to INPUT
	PORTC.DIRSET = PIN3_bm;
	
	// Setup USART
	PORTA.DIRSET = PIN2_bm;
	PORTMUX.USARTROUTEA |= PORTMUX_USART0_ALT2_gc;
	USART0.BAUD = USART0_BAUD_RATE(1200); // set BAUD RATE
	USART0.CTRLC = 0b00000011; // set "send 8bits per frame"
	USART0.CTRLB = 0b11000000; // enable Tx and Rx
	
	
	// Setup RTC
	while (RTC.STATUS > 0);
	RTC.CLKSEL |= RTC_CLKSEL_XTAL32K_gc;
	//RTC.CTRLA |= RTC_PRESCALER_DIV32768_gc;
	RTC.CTRLA |= RTC_PRESCALER_DIV1_gc;
	RTC.CTRLA |= RTC_RUNSTDBY_bm;
	RTC.CMP = 0xFFFF;
	RTC.DBGCTRL |= RTC_DBGRUN_bm;
	RTC.INTCTRL |= RTC_OVF_bm | RTC_CMP_bm;
	while(RTC.STATUS);
	RTC.CTRLA |= RTC_RTCEN_bm;
	
	// Setup PIT function in RTC	
	RTC.PITCTRLA |= RTC_PERIOD_CYC32768_gc;
	RTC.PITINTCTRL = RTC_PI_bm;
	RTC.PITCTRLA |= RTC_PITEN_bm;
	RTC.CALIB = 0x38;
	
	// SPI Display Setup
	SPI_Display spiDisplay = SPI_Display();
	spiDisplay.init_LCD();
	spiDisplay.ClearLCD();
	spiDisplay.DispPic(background);
	setTime(&spiDisplay, 0);
	
	int currTimeInSec = 0;
	volatile uint16_t count; 
    while(1)
    {
		USART0.TXDATAH = 0x00;
		count = RTC.CNT;
		USART0.TXDATAL = count; 
		while(!(USART0.STATUS | USART_TXCIF_bm));
		if(RTC.PITINTFLAGS & RTC_PI_bm)
		{
			RTC.PITINTFLAGS = 1;
			currTimeInSec++;
			setTime(&spiDisplay, currTimeInSec);
		}
    }
}

