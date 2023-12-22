/*
 * Timer.cpp
 *
 * Created: 2023-03-30 오후 1:49:28
 *  Author: Knowblesse
 */ 
#include "Timer.h"
#include <avr/io.h>
#include <stdint-gcc.h>
Timer::Timer()
{
	seconds = 0;
	isCountUp = true;
	isEnabled = Status_Initial;
	rtc_ovf_reached = false;
	linked = 0;
	isLightAlarm = true;
	isSoundAlarm = false;
	}

Button::Button(register8_t* reg, uint8_t pinNum)
{
	prevB = false;
	currB = false;
	lastRiseTime = 0;
	this->reg = reg;
	this->pinNum = pinNum;
}

bool Button::readButton(uint16_t currentTCA)
{
	bool output = false; // final output
	currB = !( (*reg & (0x01<<pinNum)) >> pinNum); // pin is set as pull-up. so should read with `!`
	if((!prevB) && currB) // Rise
	{
		lastRiseTime = currentTCA;
	}
	if(prevB && !currB) // Fall
	{
		if(currentTCA - lastRiseTime > 10)
		{
			output = true;
		}
	}
	
	prevB = currB;
	return output;
}