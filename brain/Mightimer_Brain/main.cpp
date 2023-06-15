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
#include "Timer.h"
#define USART0_BAUD_RATE(BAUD_RATE) ((float)(F_CPU * 64 / (16 * (float)BAUD_RATE)) + 0.5)
#define TCA0_CLOCK(MS) ((1000.0f / (2000000.0f / 1024.0f)) * (float)MS)

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

enum Mode
{
	Mode_Normal,
	Mode_SetTime,
	Mode_SetAdvance
};

int main(void)
{
	// Select Main Clock as internal high-freq oscillator
	uint8_t temp = CLKCTRL.OSCHFCTRLA;
	temp = CLKCTRL_FRQSEL_2M_gc; // Internal high-freq oscillator to 2MHz
	CPU_CCP = CCP_IOREG_gc;
	CLKCTRL.OSCHFCTRLA = temp;
	
	temp = CLKCTRL.MCLKCTRLA;
	temp = CLKCTRL_CLKSEL_OSCHF_gc; // Internal high-freq oscillator
	CPU_CCP = CCP_IOREG_gc;
	CLKCTRL.MCLKCTRLA = temp;
	
	
	// Check if main clock is synced
	while (CLKCTRL.MCLKSTATUS & CLKCTRL_SOSC_bm);
	
	PORTA.OUTSET = PIN3_bm;

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
	
	PORTA.OUTCLR = PIN3_bm;
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
	
	// Setup Voltage Reference
	//VREF.ADC0REF = VREF_REFSEL_VDD_gc; // Internal VDD (3.3V) is the max value
	
	// Setup ADC
	//ADC0.CTRLA = ADC_CONVMODE_SINGLEENDED_gc | ADC_RESSEL_10BIT_gc;
	//ADC0.CTRLC |= ADC_PRESC_DIV2_gc;
	//ADC0.CTRLD |= ADC_INITDLY_DLY32_gc;
	//ADC0.MUXPOS = ADC_MUXPOS_AIN7_gc;
	
	// Setup RTC for second tracking
	while (RTC.STATUS > 0);
	RTC.CLKSEL |= RTC_CLKSEL_XTAL32K_gc;
	RTC.PER = 0xFF;
	RTC.INTCTRL |= RTC_OVF_bm;
	RTC.CTRLA |= RTC_PRESCALER_DIV128_gc;
	RTC.CALIB = 0b00000000;
	RTC.CTRLA |= RTC_CORREN_bm;
	while(RTC.STATUS);
	RTC.CTRLA |= RTC_RTCEN_bm;
	
	// Setup TCA (Counter A) for millis function
	TCA0.SINGLE.PER = 0xFFFF;
	TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV1024_gc | TCA_SINGLE_ENABLE_bm; // CLK_PER = 2MHz, Clock = 2MHz/1024 = 1/1953.125 = 0.512ms
	
	// SPI Display Setup
	SPI_Display spiDisplay = SPI_Display();
	spiDisplay.init_LCD();
	spiDisplay.ClearLCD();
	spiDisplay.DispPic(background);
	setTime(&spiDisplay, 0);
	
	// Default UI Setup
	Timer timer[3] = {Timer(), Timer(), Timer()};
	
	bool prevR1 = false;
	bool prevR2 = false;
	bool currR1;
	bool currR2;
	
	long currTimeInSec = 0;
	
	int currentMode = Mode_Normal;
	int currentTab = 0;
	
	/* Button Press Logic
	// detect fall
	// get time between rise and fall
	// If the time between rise and fall is big enough, than register as a vaild click
	*/
	uint16_t currentTCA = 0;
	bool prevB2 = false;
	bool currB2 = false;
	uint16_t lastRiseTime_B2 = 0;
	
    while(1)
    {
		currentTCA = TCA0.SINGLE.CNT;
		switch (currentMode)
		{
			case Mode_Normal:
			if (timer[currentTab].isEnabled)
			{
				// update current screen
				
				
				
				// Check 
			}
			else
			{
				// Check Button2 Press
				currB2 = getBTN2();
				if(!prevB2 & currB2) // Rise
				{
					lastRiseTime_B2 = TCA0.SINGLE.CNT;
				}
				if(prevB2 & !currB2) // Fall
				{
					if(currentTCA - lastRiseTime_B2 > 10)
					{
						// Start Timer
						timer[currentTab].isEnabled = true;
						timer[currentTab].RTC_start_count = RTC.CNT;
						
					}
				}
				
			}
			
			// Check if screen should be updated
			if(timer[currentTab].isEnabled & (RTC.INTFLAGS & RTC_OVF_bm) & (RTC.CNT > timer[currentTab].RTC_start_count))
			{
				RTC.INTFLAGS |= RTC_OVF_bm;
				setTime(&spiDisplay, currTimeInSec);
				currTimeInSec++;
			}
			
			break;
			case Mode_SetTime:
			// check buttons
			break;
			case Mode_SetAdvance:
			// check buttons
			break;
		}
		
		setLED(timer[currentTab].isEnabled);
		
		
				
		
		
		
		//currR1 = getBTN1_R1();
		//currR2 = getBTN1_R2();
		//
		//if (prevR1 | prevR2)
		//{
			//if (currR1 & currR2)
			//{
				//if (prevR1 & (!prevR2))
				//{
					//currTimeInSec++;
					//setTime(&spiDisplay, currTimeInSec);	
				//}
				//else if ((!prevR1) & prevR2)
				//{
					//currTimeInSec--;
					//setTime(&spiDisplay, currTimeInSec);	
				//}
				//
			//}	
		//}	
		//
		//
		//prevR1 = currR1;
		//prevR2 = currR2;
			
    }
}

