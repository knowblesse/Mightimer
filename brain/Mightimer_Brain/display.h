/*
 * display.h
 *
 * Created: 2023-02-22 오후 5:35:19
 *  Author: Knowblesse
 */ 


#ifndef DISPLAY_H_
#define DISPLAY_H_

class SPI_Display
{
	private:
		int a;
	public:
		SPI_Display();
		void init_LCD();
		void setDataMode(bool a);
		void data_write(unsigned char d);
		void comm_write(unsigned char d);
		void setDot();
		void DispPic(const unsigned char *lcd_string);
		void DispPic(const unsigned char *picData, int startPage, int height, char startColumn, int width);
		void ClearLCD();
		void setHour(int hour);
		void setMinute(int minute);
		void setSecond(int second);
		void setArrow(int timer, int shape);
		void setPower(int power);
};

#endif /* DISPLAY_H_ */