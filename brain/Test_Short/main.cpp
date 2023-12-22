/*
 * GccApplication1.cpp
 *
 * Created: 2023-03-28 오후 12:30:08
 * Author : Knowblesse
 */ 

#define F_CPU 4000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

//#include <avr/builtins.h>


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

bool test = false;
bool PA2State = false;
ISR(PORTA_PORT_vect)
{
	test = !test;
	PORTA.INTFLAGS |= PIN5_bm;
	setLED(test);
}

ISR(TCB0_INT_vect)
{
	if (TCB0.INTFLAGS & TCB_CAPT_bm)
	{
		TCB0.INTFLAGS |= TCB_CAPT_bm;
		if (PA2State)
		{
			PORTA.OUTCLR = PIN2_bm;
			PA2State = false;
		}
		else
		{
			PORTA.OUTSET = PIN2_bm;
			PA2State = true;
		}
	}
}


int main(void)
{
	uint8_t temp;

	// Select Main Clock as internal high-freq oscillator
	temp = CLKCTRL.OSCHFCTRLA;
	temp = CLKCTRL_FRQSEL_2M_gc; // Internal high-freq oscillator to 2MHz
	CPU_CCP = CCP_IOREG_gc;
	CLKCTRL.OSCHFCTRLA = temp;
	
	temp = CLKCTRL.MCLKCTRLA;
	temp = CLKCTRL_CLKSEL_OSCHF_gc; // Internal high-freq oscillator
	CPU_CCP = CCP_IOREG_gc;
	CLKCTRL.MCLKCTRLA = temp;
	
		
	// Check if main clock is synced
	while (CLKCTRL.MCLKSTATUS & CLKCTRL_SOSC_bm);
	
	// Setup Ports	
	/*
	PF6 : BTN2 PUSH
	PA2 : LED
	PA3 : LCD LED
	PA4 : SPI_COPI
	PA5 : BTN1 PUSH
	PA6 : SPI_CLK
	PA7 : SPI_CS
	
	PC1 : LCD_RESET
	PC2 : LCD_A0
	PC3 : BTN2-R2
	
	PD4 : BTN2-R1
	PD5 : BTN1-R2
	PD6 : BTN1-R1
	PD7 : BATT_LEVEL
	*/
	
	
	
	PORTF.DIRCLR = PIN6_bm; // BTN2_PUSH
	PORTF.PIN6CTRL |= PORT_PULLUPEN_bm;
	
	PORTA.DIRSET = PIN2_bm | PIN3_bm; // LED, LCD_LED
	
	PORTA.DIRCLR = PIN5_bm; // BTN1_PUSH
	PORTA.PIN5CTRL |= PORT_PULLUPEN_bm | PORT_ISC_RISING_gc;
	
	//PORTA.OUTSET = PIN2_bm;
	
	// Set TCB
 	TCB0.CTRLA |= TCB_CLKSEL_DIV1_gc | TCB_ENABLE_bm;
 	TCB0.CTRLB |= TCB_CNTMODE_INT_gc;
	TCB0.INTCTRL |= TCB_CAPT_bm;
 	TCB0.CCMP = 0x00F5;
	
	
	
	sei();
	while(1)
	{

	}

}

