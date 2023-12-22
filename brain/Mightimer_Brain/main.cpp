/*
 * Mightimer_Brain.cpp
 *
 * Created: 2023-02-22 오후 5:41:37
 * Author : Knowblesse
 */ 

#define F_CPU 4000000UL

#ifdef __cplusplus
extern "C" {
	#endif
	#include <avr/io.h>
	#include <avr/cpufunc.h>
	#ifdef __cplusplus
}
#endif

#include <avr/interrupt.h>
#include <util/delay.h>
#include <time.h>
#include "display.h"
#include "heximage.h"
#include "Timer.h"

#define TCA0_CLOCK(MS) ((1000.0f / (2000000.0f / 1024.0f)) * (float)MS)

void setLED(bool state){
	if (state) PORTA.OUTSET = PIN3_bm;
	else PORTA.OUTCLR = PIN3_bm;
}

double getBattState(){
	ADC0.CTRLA |= ADC_ENABLE_bm;
	ADC0.COMMAND |= ADC_STCONV_bm;
	while(ADC0.COMMAND & ADC_STCONV_bm);
	ADC0.CTRLA &= ~ADC_ENABLE_bm;
	double voltage = ((((double)ADC0.RES / 1024.0) * 3.3) * 2);
	return (voltage - 3.6) / 0.8 * 100; // 3.6V = 0%, 4.2% = 100%
}

enum Mode{
	Mode_Normal,
	Mode_SetTime,
	Mode_SetAdvance
};

// Global Variables
int currentMode = Mode_Normal;
int currSecond = -1;
int currMinute = -1;
int currHour = -1;
volatile int BtnL_inc = 0;
volatile int BtnR_inc = 0;
volatile bool BtnR_pushed = false;
volatile int allInt = 0;
volatile uint16_t rotationTCA = 0;
volatile int numRTC_OVR_cnt = 0;

void setTime(SPI_Display *spiDisplay, long long timeInSec){
	int hour = (int)((double)timeInSec / 3600.0);
	int minute = (int)((double)(timeInSec - 3600*hour) / 60.0);
	int second = timeInSec % 60;
	
	if(currSecond != second) spiDisplay->setSecond(second);
	if(currMinute != minute) spiDisplay->setMinute(minute);
	if(currHour != hour) spiDisplay->setHour(hour);
}

// ISR for rotation event
ISR(PORTD_PORT_vect){
	// First, check debounce
	if (TCA0.SINGLE.CNT - rotationTCA > 40){
		//Then, check `currentMode`
		if (currentMode == Mode_Normal){
			// If a rotation occurred during normal mode, go to set time mode
			currentMode = Mode_SetTime;
		}
		else if (currentMode == Mode_SetTime){
			// If a rotation occurred during set time mode, 
			// Check which encoder evoked the interrupt
			// Left one
			if (PORTD.INTFLAGS & PIN6_bm){
				if (!(PORTD.IN & PIN5_bm)) BtnL_inc++; // R1 is lagging
				else BtnL_inc--;			
			}
			// Right one
			else{
				if (!(PORTC.IN & PIN3_bm)) BtnR_inc++; // R1 is lagging
				else BtnR_inc--;
			}
		}
		rotationTCA = TCA0.SINGLE.CNT; // set the new time for the debounce
	}
	PORTD.INTFLAGS = PORTD.INTFLAGS; // clear flag
}

ISR(PORTF_PORT_vect){
	if(PORTF.INTFLAGS & PIN6_bm){
		BtnR_pushed = true;
	}
	PORTF.INTFLAGS = PORTF.INTFLAGS; // clear flag
}
	
ISR(RTC_CNT_vect)
{
	numRTC_OVR_cnt++;
	RTC.INTFLAGS |= RTC_OVF_bm;
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

	// Set External 32kHz oscillator
	temp = CLKCTRL.XOSC32KCTRLA;
	temp &= ~CLKCTRL_ENABLE_bm;
	ccp_write_io((uint8_t *) &CLKCTRL.XOSC32KCTRLA, temp);

	
	while(CLKCTRL.MCLKSTATUS & CLKCTRL_XOSC32KS_bm){}
	
	temp &= ~CLKCTRL_SEL_bm; // select i'm using two pins for oscillator
	
	temp = CLKCTRL.XOSC32KCTRLA;
	temp |= CLKCTRL_ENABLE_bm;
	temp |= CLKCTRL_RUNSTBY_bm; // always running. maybe not necessary
	ccp_write_io((uint8_t *) &CLKCTRL.XOSC32KCTRLA, temp);
	
	while (!(CLKCTRL.MCLKSTATUS & CLKCTRL_XOSC32KS_bm));	
	
	
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
	PORTF.PIN6CTRL |= PORT_PULLUPEN_bm | PORT_ISC_RISING_gc;
	
	PORTA.DIRSET = PIN2_bm | PIN3_bm; // LED, LCD_LED
	
	PORTA.DIRCLR = PIN5_bm; // BTN1_PUSH
	PORTA.PIN5CTRL |= PORT_PULLUPEN_bm;
	
	PORTC.DIRCLR = PIN3_bm; // BTN2_R2
	PORTC.PIN3CTRL |= PORT_PULLUPEN_bm;
	
	PORTD.DIRCLR = PIN4_bm | PIN5_bm | PIN6_bm; // BTN2_R1, BTN1_R2, BTN1_R1
	PORTD.PIN4CTRL = PORT_PULLUPEN_bm | PORT_ISC_FALLING_gc; // BTN2_R1
	PORTD.PIN5CTRL = PORT_PULLUPEN_bm;
	PORTD.PIN6CTRL = PORT_PULLUPEN_bm | PORT_ISC_FALLING_gc; // BTN1_R1
	
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
	RTC.DBGCTRL |= RTC_DBGRUN_bm;
	RTC.PER = 0xFF;
	RTC.INTCTRL |= RTC_OVF_bm;
	RTC.CTRLA |= RTC_PRESCALER_DIV128_gc;
	RTC.CALIB = 0b00000000;
	RTC.CTRLA |= RTC_CORREN_bm;
	while(RTC.STATUS);
	RTC.CTRLA |= RTC_RTCEN_bm;
	
	// Setup TCA (Counter A) for millis function
	TCA0.SINGLE.PER = 0xFFFF;
	TCA0.SINGLE.CTRLA = TCA_SINGLE_DBGRUN_bm | TCA_SINGLE_CLKSEL_DIV1024_gc | TCA_SINGLE_ENABLE_bm; // CLK_PER = 2MHz, Clock = 2MHz/1024 = 1/1953.125 = 0.512ms
	
	// SPI Display Setup
	SPI_Display spiDisplay = SPI_Display();
	spiDisplay.init_LCD();
	spiDisplay.ClearLCD();
	spiDisplay.DispPic(background);
	setTime(&spiDisplay, 0);
	
	// Default UI Setup
	Timer timer[3] = {Timer(), Timer(), Timer()};

	long currTabLastSecond = 0;
	
	long currTimeInSec = 0;
	
	int currentTab = 0;
	
	/* Button Press Logic
	// detect fall
	// get time between rise and fall
	// If the time between rise and fall is big enough, than register as a valid click
	*/
	volatile uint16_t currentTCA;
	
	// Clicks
	Button Btn_L = Button(&PORTA.IN, 5);
	Button Btn_R = Button(&PORTF.IN, 6);

	sei();
	
	uint16_t t;
    
	while(1){
		currentTCA = TCA0.SINGLE.CNT;
		Btn_L.readButton(currentTCA);
		
	
		t = RTC.CNT;
		// Check Count
		if (numRTC_OVR_cnt > 0){
			if (timer[currentTab].isEnabled > 1){
				timer[currentTab].rtc_ovf_reached = true;
			}
			numRTC_OVR_cnt = 0;
		}
	
		// Check CNT
		if (timer[currentTab].isEnabled > 1 && timer[currentTab].rtc_ovf_reached){
			if (t >= timer[currentTab].time_criterion){
			timer[currentTab].seconds++;
			timer[currentTab].rtc_ovf_reached = false;
			}
		}
	
		// Check Mode
		switch (currentMode){
			case Mode_Normal:
				// In the normal mode, the currently showing timer can either be moving or not.
				
				//TODO: Change this into interrupt
				if(BtnR_pushed){// Right button is pushed. 
					BtnR_pushed = false;
					switch (timer[currentTab].isEnabled){
						case Status_Initial: // The initial state of the timer
							timer[currentTab].isEnabled = Status_FirstMoving;
							timer[currentTab].time_criterion = RTC.CNT;
							setLED(true);
							break;
						case Status_Paused: // If the timer was paused, it can be resumed
							timer[currentTab].isEnabled = Status_Resumed;
							timer[currentTab].time_criterion = RTC.CNT + timer[currentTab].time_criterion;
							setLED(true);
							break;
						case Status_FirstMoving: // If the timer was first moving, 
						case Status_Resumed: // or if the timer was resumed,
							//it can be paused
							//or time limit can be reached (count-down only)
							t = RTC.CNT;
							timer[currentTab].isEnabled = Status_Paused;
							timer[currentTab].time_criterion = 0xFF - (t - timer[currentTab].time_criterion);
							setLED(false);
							break;
					}
				}
	
				// Check if screen should be updated
				if(timer[currentTab].seconds != currTabLastSecond){
					setTime(&spiDisplay, timer[currentTab].seconds);
				}
				break;
	
			case Mode_SetTime:
				setLED(true);
				// If rotation interrupt has occurred, and BtnR_inc is not zero,
				// apply the changed value into the currTimeInSec and reset BtnR_inc
				if (BtnR_inc != 0){
					currTimeInSec += BtnR_inc;
					if(currTimeInSec < 0) currTimeInSec = 0;
					BtnR_inc = 0;
					setTime(&spiDisplay, currTimeInSec);
				}
				
				if(BtnR_pushed){
					BtnR_pushed = false;
					currentMode = Mode_Normal;
				}
	
				// Check if screen should be updated
				break;
			case Mode_SetAdvance:
				// check buttons
				break;
			}
		}
	
}







