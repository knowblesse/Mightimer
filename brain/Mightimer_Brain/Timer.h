/*
 * Timer.h
 *
 * Created: 2023-03-30 오후 1:53:58
 *  Author: Knowblesse
 */ 


#ifndef TIMER_H_
#define TIMER_H_
#include <stdint-gcc.h>
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




#endif /* TIMER_H_ */