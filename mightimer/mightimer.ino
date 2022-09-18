
/*
 * PIN 1 : CLK
 * PIN 2 : DATA
 * PIN 3 : V+
 * PIN 4 : A0
 * PIN 5 : RESET
 * PIN 6 : CS
 */

/****************************************************
 *               Pinout on Arduino Uno               
 *****************************************************/
#include <SPI.h>
#include "heximage.h"
#define RES 4 // Reset signal
#define CS 3  // Chip select signal
#define RS 2 // Register select signal
#define SC 13 // Serial clock signal
#define SI 11 // Serial data signal
#define LED 5

/****************************************************
 *                 Function Commands                 
 *****************************************************/

void data_write(unsigned char d) // Data Output Serial Interface
{
  unsigned int n;
  digitalWrite(CS, LOW);
  digitalWrite(RS, HIGH);
  
  SPI.transfer(d);
  
  digitalWrite(CS, HIGH);
}

void comm_write(unsigned char d) // Command Output Serial Interface
{
  unsigned int n;
  digitalWrite(CS, LOW);
  digitalWrite(RS, LOW);

  SPI.transfer(d);

  digitalWrite(CS, HIGH);
}

void DispPic(unsigned char *lcd_string)
{
  unsigned int i, j;
  unsigned char page = 0xB0;
  //comm_write(0xAE); // Display OFF
  comm_write(0x40); // Display start address + 0x40
  for (i = 0; i < 8; i++)
  {                   // 64 pixel display / 8 pixels per page = 8 pages
    comm_write(page); // send page address
    comm_write(0x10); // column address upper 4 bits + 0x10
    comm_write(0x00); // column address lower 4 bits + 0x00
    for (j = 0; j < 128; j++)
    {                          // 128 columns wide
      data_write(*lcd_string); // send picture data
      lcd_string++;
    }
    page++; // after 128 columns, go to next page
  }
  //comm_write(0xAF);
}

void DispPic(unsigned char *picData, int startPage, int height, int startColumn, int width)
{
  // height : page height -> 8 pixel = 1 page = 1 height
  unsigned int i, j;
  comm_write(0x40); // Display start address
  for (int page = startPage; page < startPage + height; page++)
  {
    comm_write(0xB0 + (unsigned char)page); // send page address
    comm_write(0x10 + ((byte)startColumn >> 4)); // column address upper 4 bits 
    comm_write((byte)startColumn & 0b00001111); // column address lower 4 bits
    for (int col = 0; col < width; col++)
    {                          // 128 columns wide
      data_write(*picData); // send picture data
      picData++;
    }
  }
}

void ClearLCD(unsigned char *lcd_string)
{
  unsigned int i, j;
  unsigned char page = 0xB0;
  comm_write(0xAE); // Display OFF
  comm_write(0x40); // Display start address + 0x40
  for (i = 0; i < 8; i++)
  {                   // 64 pixel display / 8 pixels per page = 8 pages
    comm_write(page); // send page address
    comm_write(0x10); // column address upper 4 bits + 0x10
    comm_write(0x00); // column address lower 4 bits + 0x00
    for (j = 0; j < 128; j++)
    {                   // 128 columns wide
      data_write(0x00); // write clear pixels
      lcd_string++;
    }
    page++; // after 128 columns, go to next page
  }
  comm_write(0xAF);
}

/****************************************************
 *           Initialization For controller           
 *****************************************************/

void init_LCD()
{
  comm_write(0xAE); // Display OFF 0b10101110
  
  comm_write(0xA2); // LCD bias set (11) 0b10100010
  comm_write(0xA0); // ADC select (8) 0b10100000
  comm_write(0xC8); // COM output mode (15) reversed 0b11001000

  //110
  comm_write(0b0010000 + 4); // Resistor Ratio Set (17)00100XXX 0b00100110
  comm_write(0x81); // Electronic Volume Command (set contrast) Double Byte: 1 of 2 0b10000001
  comm_write(35); // Electronic Volume value (contrast value) Double Byte: 2 of 2 0b00010001

  comm_write(0x2F); // Power Control set (all on)
  
  comm_write(0xAF); // Display ON
}

void setHour(int hour)
{
  DispPic(large_digit[hour], 2, 5, 0, 27);
}

void setMinute(int minute)
{
  int first_digit = (int)(minute / 10);
  int last_digit = minute % 10;
  DispPic(large_digit[first_digit], 2, 5, 37, 27);
  DispPic(large_digit[last_digit], 2, 5, 64, 27);
}

void setSecond(int second)
{
  int first_digit = (int)(second / 10);
  int last_digit = second % 10;
  DispPic(small_digit[first_digit], 5, 3, 91, 18);
  DispPic(small_digit[last_digit], 5, 3, 110, 18);
}
/*****************************************************
 *           Setup Function, to run once              
 *****************************************************/

void setup()
{
  pinMode(RES, OUTPUT); // configure RES as output
  pinMode(CS, OUTPUT);  // configure CS as output
  pinMode(RS, OUTPUT);  // configure RS as output
  pinMode(SC, OUTPUT);  // configure SC as output
  pinMode(SI, OUTPUT);  // configure SI as output
  pinMode(LED, OUTPUT);

  SPI.begin();
  SPI.beginTransaction(SPISettings(3000000, MSBFIRST, SPI_MODE3));
  
  digitalWrite(5, HIGH);
  digitalWrite(RES, LOW);
  delay(100);
  digitalWrite(RES, HIGH);
  delay(100);
  Serial.begin(9600);
  init_LCD();
  DispPic(background);
  setHour(1);
  setMinute(23);
}



/*****************************************************
 *           Loop Function, to run repeatedly         
 *****************************************************/

int hour = 0;
int minute = 0;
unsigned long currTime;
void loop()
{
  currTime = millis();
  if(currTime%100 < 10)
  {
     setHour(0);
     setMinute((int)(currTime / (60*100)));
     setSecond( (int)((int)currTime % 6000) / 100 );
     delay(10);
  }
}
