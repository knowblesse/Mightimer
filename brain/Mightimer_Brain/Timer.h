/*
 * Timer.h
 *
 * Created: 2023-03-30 오후 1:53:58
 *  Author: Knowblesse
 */ 


#ifndef TIMER_H_
#define TIMER_H_

class Timer
{
	public:
		long seconds = 0;
		uint16_t RTC_start_counter;
		bool isCountUp = true;
		bool isEnabled = false;
		int linked = 0;
		bool isLightAlarm = true;
		bool isSoundAlarm = false;
		
		void start();
		void stop();
	};




#endif /* TIMER_H_ */