
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
#include "display.h"
#define RES 4 // Reset signal
#define CS 3  // Chip select signal
#define RS 2 // Register select signal
#define SC 13 // Serial clock signal
#define SI 11 // Serial data signal
#define LED 5

/****************************************************
 *                 Function Commands                 
 *****************************************************/

class Timer
{
  public:
  bool isCountUp;
  bool isAlive;
  int alarmType; // 0 : none, 1 : light, 2 : sound
  int setTime;
  unsigned long startTime;

  public:
  Timer(){};
  void changeDirection()
  {
    isCountUp = !isCountUp;
    isAlive = !isAlive;
  };
};

Timer timer[3];

/*****************************************************
 *           Setup Function, to run once              
 *****************************************************/

void setup()
{
  // Initialize Timer
  for(int i = 0; i < 3; i++)
  {
    timer[i].isCountUp = true;
    timer[i].isAlive = false;
    timer[i].alarmType = 0;
    timer[i].setTime = 0;
    timer[i].startTime = 0;
  }
  
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
