#ifndef DISPLAY_H
#define DISPLAY_H


void data_write(unsigned char d);
void comm_write(unsigned char d);
void DispPic(const unsigned char *lcd_string);
void DispPic(const unsigned char *picData, int startPage, int height, int startColumn, int width);
void ClearLCD(unsigned char *lcd_string);
void init_LCD();
void setHour(int hour);
void setMinute(int minute);
void setSecond(int second);

#endif
