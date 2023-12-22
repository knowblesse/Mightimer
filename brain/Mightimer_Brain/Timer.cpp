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
	isEnabled = Status_Stop;
	rtc_ovf_reached = false;
	linked = 0;
	isLightAlarm = true;
	isSoundAlarm = false;
	}

Button::Button(register8_t* reg, uint8_t pinNum)
{
	prevB = false;
	currB = false;
	state = false;
	lastRiseTime = 0;
	this->reg = reg;
	this->pinNum = pinNum;
}

bool Button::readButton(uint16_t currentTCA)
{
	bool output = false;
	currB = !( (*reg & (0x01<<pinNum)) >> pinNum);
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
	state = output;	
	return output;
}

Encoder::Encoder(register8_t* reg1, uint8_t pinNum1, register8_t* reg2, uint8_t pinNum2)
{
	prevR1 = false;
	prevR2 = false;
	currR1 = false;
	currR2 = false;
	
	this->reg1 = reg1;
	this->reg2 = reg2;
	
	this->pinNum1 = pinNum1;
	this->pinNum2 = pinNum2;
	
	state = 0;
	
	test = false;
}

bool Encoder::readEncoder()
{
	currR1 = !( (*reg1 & (0x01<<pinNum1)) >> pinNum1);
	currR2 = !( (*reg2 & (0x01<<pinNum2)) >> pinNum2);
	
	state = 0;
	
	if (test == false && currR1 == true)
	{
		test = true;
	}
	
	if (test == true && currR1 == false)
	{
		if (currR2 == true)
		{
			state = 1;	
		}
		else
		{
			state = -1;
		}
		test = false;
	}
		
	prevR1 = currR1;
	prevR2 = currR2;
	
	return state;
}