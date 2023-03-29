/*
 * GccApplication1.cpp
 *
 * Created: 2023-03-28 오후 12:30:08
 * Author : Knowblesse
 */ 

#define F_CPU 4000000UL
#include <avr/io.h>
#include <util/delay.h>
#include "lib1.h"

bool getBTN1() {return !(PORTA.IN & PIN5_bm);}
bool getBTN2() {return !(PORTF.IN & PIN6_bm);}
void setLED(bool state)
{
	if (state)
	{
		PORTA.OUTSET = PIN2_bm;
		PORTA.OUTSET = PIN3_bm;
	}
	else 
	{
		PORTA.OUTCLR = PIN2_bm;
		PORTA.OUTCLR = PIN3_bm;
	}
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
	
	/* Replace with your application code */
	PORTF.DIRCLR = PIN6_bm; // BTN2_PUSH
	PORTF.PIN6CTRL |= PORT_PULLUPEN_bm;
	    
	PORTA.DIRSET = PIN2_bm | PIN3_bm; // LED, LCD_LED
	    
	PORTA.DIRCLR = PIN5_bm; // BTN1_PUSH
	PORTA.PIN5CTRL |= PORT_PULLUPEN_bm;
	    
	setLED(true);
	_delay_ms(1000);
	setLED(false);

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

		setLED(true);
		_delay_ms(1000);
		setLED(false);

    while (1) 
    {
		setLED(getBTN1() | getBTN2());
    }
}

