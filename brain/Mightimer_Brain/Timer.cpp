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
	long seconds = 0;
	uint16_t RTC_start_counter;
	bool isCountUp = true;
	bool isEnabled = false;
	int linked = 0;
	bool isLightAlarm = true;
	bool isSoundAlarm = false;
	
	
	};