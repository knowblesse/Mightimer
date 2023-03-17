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
	
	// Select Main Clock as internal high-freq oscillator
	uint8_t temp = CLKCTRL.MCLKCTRLA;
	temp = CLKCTRL_CLKSEL_OSCHF_gc; // Internal high-freq oscillator
	CPU_CCP = CCP_IOREG_gc;
	CLKCTRL.MCLKCTRLA = temp;
	
	// Check if main clock is synced
	while (CLKCTRL.MCLKSTATUS & CLKCTRL_SOSC_bm);

	// Set External 32kHz oscillator
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
	
	// Setup Ports	
	PORTA.DIRSET = PIN0_bm | PIN3_bm; //pin PA0 (Tx), PA3(LED) to OUTPUT
	PORTA.DIRCLR = PIN1_bm; //pin PA1(Rx) to INPUT
	PORTC.DIRSET = PIN3_bm;
	
	// Setup USART
	PORTA.DIRSET = PIN2_bm;
	PORTMUX.USARTROUTEA |= PORTMUX_USART0_ALT2_gc;
	USART0.BAUD = USART0_BAUD_RATE(9600); // set BAUD RATE
	USART0.CTRLC = 0b00000011; // set "send 8bits per frame"
	USART0.CTRLB = 0b11000000; // enable Tx and Rx
	
	// Setup Voltage Reference
	VREF.ADC0REF = VREF_REFSEL_VDD_gc; // Internal VDD (3.3V) is the max value
	
	// Setup ADC
	ADC0.CTRLA = ADC_CONVMODE_SINGLEENDED_gc | ADC_RESSEL_10BIT_gc;
	ADC0.CTRLC |= ADC_PRESC_DIV2_gc;
	ADC0.CTRLD |= ADC_INITDLY_DLY32_gc;
	ADC0.MUXPOS = ADC_MUXPOS_AIN31_gc; //PC3 == AIN31
	
	
	// Setup RTC
	while (RTC.STATUS > 0);
	RTC.CLKSEL |= RTC_CLKSEL_XTAL32K_gc;
	RTC.PER = 0xFF;
	RTC.INTCTRL |= RTC_OVF_bm;
	RTC.CTRLA |= RTC_PRESCALER_DIV128_gc;
	RTC.CALIB = 0b00000000;
	RTC.CTRLA |= RTC_CORREN_bm;
	while(RTC.STATUS);
	RTC.CTRLA |= RTC_RTCEN_bm;
	
	// SPI Display Setup
	SPI_Display spiDisplay = SPI_Display();
	spiDisplay.init_LCD();
	spiDisplay.ClearLCD();
	spiDisplay.DispPic(background);
	setTime(&spiDisplay, 0);
	
	int currTimeInSec = 0;
	volatile uint16_t count; 
	double val;
	int large;
    while(1)
    {
		if(RTC.INTFLAGS & RTC_OVF_bm)
		{
			RTC.INTFLAGS |= RTC_OVF_bm;
			//setTime(&spiDisplay, currTimeInSec);
			currTimeInSec++;
			
			if((currTimeInSec % 10) == 0)
			{
				ADC0.CTRLA |= ADC_ENABLE_bm;
				ADC0.COMMAND |= ADC_STCONV_bm;
				while(ADC0.COMMAND & ADC_STCONV_bm);
				//while(!(ADC0.INTFLAGS & ADC_RESRDY_bm));
				val = (((double)ADC0.RES / 1024.0) * 3.3);
				large = (int)val;
				spiDisplay.setHour(large);
				spiDisplay.setMinute((int)(val*100) - large*100);
				
				ADC0.CTRLA &= ~ADC_ENABLE_bm;	
			}
		}
    }
}

