/*
 * Mightimer_Brain.cpp
 *
 * Created: 2023-02-22 오후 5:41:37
 * Author : Knowblesse
 */ 

#define F_CPU 4000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <time.h>
#include "display.h"
#include "heximage.h"
#define USART0_BAUD_RATE(BAUD_RATE) ((float)(F_CPU * 64 / (16 * (float)BAUD_RATE)) + 0.5)

int currSecond = -1;
int currMinute = -1;
int currHour = -1;

bool getBTN1() {return !(PORTA.IN & PIN5_bm);}
bool getBTN2() {return !(PORTF.IN & PIN6_bm);}
bool getBTN1_R1() {return !(PORTD.IN & PIN6_bm);}
bool getBTN1_R2() {return !(PORTD.IN & PIN5_bm);}
bool getBTN2_R1() {return !(PORTD.IN & PIN4_bm);}
bool getBTN2_R2() {return !(PORTC.IN & PIN3_bm);}
void setLED(bool state)
{
	if (state) PORTA.OUTSET = PIN3_bm;
	else PORTA.OUTCLR = PIN3_bm;
}

double getBattState()
{
	ADC0.CTRLA |= ADC_ENABLE_bm;
	ADC0.COMMAND |= ADC_STCONV_bm;
	while(ADC0.COMMAND & ADC_STCONV_bm);
	ADC0.CTRLA &= ~ADC_ENABLE_bm;
	double voltage = ((((double)ADC0.RES / 1024.0) * 3.3) * 2);
	return (voltage - 3.6) / 0.8 * 100; // 3.6V = 0%, 4.2% = 100%
}


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
	PORTA.PIN5CTRL |= PORT_PULLUPEN_bm;
	
	PORTC.DIRCLR = PIN3_bm; // BTN2_R2
	PORTC.PIN3CTRL |= PORT_PULLUPEN_bm;
	
	PORTD.DIRCLR = PIN4_bm | PIN5_bm | PIN6_bm; // BTN2_R1, BTN1_R2, BTN1_R1
	PORTD.PIN4CTRL = PORT_PULLUPEN_bm;
	PORTD.PIN5CTRL = PORT_PULLUPEN_bm;
	PORTD.PIN6CTRL = PORT_PULLUPEN_bm;
	
	for(int i=0; i < 10; i++)
	{
		setLED(true);
		_delay_ms(100);
		setLED(false);
		_delay_ms(100);
	}
	
	// Setup Voltage Reference
	VREF.ADC0REF = VREF_REFSEL_VDD_gc; // Internal VDD (3.3V) is the max value
	
	// Setup ADC
	ADC0.CTRLA = ADC_CONVMODE_SINGLEENDED_gc | ADC_RESSEL_10BIT_gc;
	ADC0.CTRLC |= ADC_PRESC_DIV2_gc;
	ADC0.CTRLD |= ADC_INITDLY_DLY32_gc;
	ADC0.MUXPOS = ADC_MUXPOS_AIN7_gc;
	
	// Setup RTC
	while (RTC.STATUS > 0);
	RTC.CLKSEL |= RTC_CLKSEL_XTAL32K_gc;
	//RTC.CLKSEL |= RTC_CLKSEL_OSC32K_gc;
	RTC.PER = 0xFF;
	RTC.INTCTRL |= RTC_OVF_bm;
	RTC.CTRLA |= RTC_PRESCALER_DIV128_gc;
	RTC.CALIB = 0b00000000;
	RTC.CTRLA |= RTC_CORREN_bm;
	while(RTC.STATUS);
	RTC.CTRLA |= RTC_RTCEN_bm;
	
	// Setup TCA (Counter A)
	TCA0.SINGLE.PER = 0x00FF;
	TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV1_gc | TCA_SINGLE_ENABLE_bm;
	
	// SPI Display Setup
	SPI_Display spiDisplay = SPI_Display();
	spiDisplay.init_LCD();
	spiDisplay.ClearLCD();
	spiDisplay.DispPic(background);
	setTime(&spiDisplay, 0);
	
	int currTimeInSec = 10;
	volatile uint16_t count; 
	double val;
	int large;
	
	bool isTimerStarted = true;

	bool prevR1 = false;
	bool prevR2 = false;
	bool currR1;
	bool currR2;
	
    while(1)
    {
		// Check Buttons
		setLED(isTimerStarted);
		
		currR1 = getBTN1_R1();
		currR2 = getBTN1_R2();
		
		if (prevR1 | prevR2)
		{
			if (currR1 & currR2)
			{
				if (prevR1 & (!prevR2))
				{
					currTimeInSec++;
					setTime(&spiDisplay, currTimeInSec);	
				}
				else if ((!prevR1) & prevR2)
				{
					currTimeInSec--;
					setTime(&spiDisplay, currTimeInSec);	
				}
				
			}	
		}	
		
		
		prevR1 = currR1;
		prevR2 = currR2;
		
		if( getBTN1() & (TCA0.SINGLE.INTFLAGS & TCA_SINGLE_OVF_bm))
		{
			isTimerStarted = !isTimerStarted;
			TCA0.SINGLE.INTFLAGS = TCA_SINGLE_OVF_bm;
		}
		
		if(isTimerStarted & (RTC.INTFLAGS & RTC_OVF_bm))
		{
			RTC.INTFLAGS |= RTC_OVF_bm;
			setTime(&spiDisplay, currTimeInSec);
			currTimeInSec++;
		}
		
		if (currTimeInSec == -1)
		{
			for(int i=0; i < 10; i++)
			{
				setTime(&spiDisplay, currTimeInSec);
				setLED(true);
			_delay_ms(500);
			setLED(false);
			_delay_ms(500);	
			}
			
			isTimerStarted = false;
			currTimeInSec = 0;
		}
			
    }
}

