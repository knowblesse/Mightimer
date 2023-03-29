/*
 * GccApplication1.cpp
 *
 * Created: 2023-03-28 오후 12:30:08
 * Author : Knowblesse
 */ 

#define F_CPU 4000000UL
#include <avr/io.h>
#include <util/delay.h>
#include "lib1.h"

void blink(void)
{
	
		PORTA.OUTSET = PIN2_bm | PIN3_bm;
		_delay_ms(500);
		PORTA.OUTCLR = PIN2_bm | PIN3_bm;
		_delay_ms(500);

}

