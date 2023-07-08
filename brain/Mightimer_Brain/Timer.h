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
class Timer
{
	public:
		long seconds;
		uint16_t RTC_start_count;
		bool isCountUp;
		bool isEnabled;
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
		bool state;
		uint16_t lastRiseTime;
		
		Button(register8_t* reg, uint8_t pinNum);
		bool readButton(uint16_t currentTCA);
	};

class Encoder
{
	public:
		bool prevR1;
		bool prevR2;
		bool currR1;
		bool currR2;
		
		bool test;
		
		register8_t* reg1;
		register8_t* reg2;
		
		uint8_t pinNum1;
		uint8_t pinNum2;
		
		int state;
		
		Encoder(register8_t* reg1, uint8_t pinNum1, register8_t* reg2, uint8_t pinNum2);
		bool readEncoder();
	};


#endif /* TIMER_H_ */
