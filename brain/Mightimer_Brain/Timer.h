/*
 * Timer.h
 *
 * Created: 2023-03-30 오후 1:53:58
 *  Author: Knowblesse
 */ 


#ifndef TIMER_H_
#define TIMER_H_
#include <stdint-gcc.h>
#include <avr/io.h>

enum TimerStatus
{
	Status_Initial,
	Status_Paused,
	Status_FirstMoving,
	Status_Resumed
};

class Timer
{
	public:
		long seconds;
		uint8_t time_criterion;
		bool rtc_ovf_reached;
		bool isCountUp;
		int isEnabled;
		int linked;
		bool isLightAlarm;
		bool isSoundAlarm;
		
		void start();
		void stop();
		Timer();
	};

class Button
{
	
	public:
		register8_t* reg;
		uint8_t pinNum;
		bool prevB;
		bool currB;
		uint16_t lastRiseTime;
		
		Button(register8_t* reg, uint8_t pinNum);
		bool readButton(uint16_t currentTCA);
	};
#endif /* TIMER_H_ */
