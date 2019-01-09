/////////////////////////////////////////////////////////
/*
  Jenna deBoisblanc
  2016
  http://jdeboi.com/

  CLASSROOM CLOCK
  This customizable Arduino clock was developed for the Isidore
  Newman School Makerspace. Some of the features include:
  - tracks and displays Newman's rotating block (A, B, C...)
  - uses a red->green color gradient to show the amount of time
    remaining in the period
  - flashes between the time and a countdown timer when the end
    of the period approaches
  - rainbows during lunch and Assembly, pulses after school...
  And so much more! Add your own functions to make School Clock
  even cooler!

*/
/////////////////////////////////////////////////////////

#include "classroomClock.h"
#include <Wire.h>
#include <SPI.h>
#include "RTClib.h"               // https://github.com/adafruit/RTClib
#include <avr/power.h>
#include <Adafruit_NeoPixel.h>    // https://github.com/adafruit/Adafruit_NeoPixel

#define NEOPIXEL_PIN 3
#define NUM_PIXELS 32

#define DEBUG false

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_PIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);
RTC_DS1307 RTC;
DateTime now;
uint8_t currentMinute = 60;
uint8_t currentPeriod = 0;
uint8_t currentDay = 0;
uint8_t lastHour = 0;
uint8_t timeBlockIndex = 0;

DateTime lastFlash;    // for countdown "flash"
boolean flashOn = false;

const /*PROGMEM*/ uint8_t numbers[] = {
  B11101110,    // 0
  B10001000,    // 1
  B01111100,    // 2     5555
  B11011100,    // 3   6     4
  B10011010,    // 4     3333
  B11010110,    // 5   2     0
  B11110110,    // 6     1111
  B10001100,    // 7
  B11111110,    // 8
  B10011110     // 9
};

const /*PROGMEM*/ uint8_t letters[] = {
  B10111110,    // A      55555
  B11110010,    // B     6     4
  B01110000,    // C     6     4
  B11111000,    // D      33333
  B01110110,    // E     2     0
  B00110110,    // F     2     0
  B11011110,    // G      11111
  B10111010,    // H
  B01100010,    // L
  B00000000     //
};

enum ClockTypes
{
  ctWeekend,
  ctHoliday,
  ctBeforeSchool,
  ctAfterSchool,
  ctEndFlash,
  ctLunch,
  ctAssembly,
  ctDuringClass,
  ctPassing,
} clockType;

enum DayTypes
{
  dtSchoolDay,
  dtWeekend,
  dtHoliday,
} dayType;

/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
// COLOR CONSTANTS
/////////////////////////////////////////////////////////
#define RED          0x00FF0000
#define GREEN        0x0000FF00
#define BLUE         0x000000FF
#define PURPLE       0x00800080
#define YELLOW       0x00FFFF00
#define ORANGE       0x00FFA500
#define LUNCH        ORANGE
#define ASPIRE       ORANGE
#define OUTOFCLUSTER ORANGE


/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
// SCHEDULE RELATED STRUCTURE DEFINITIONS
/////////////////////////////////////////////////////////
typedef struct _Period
{
  uint8_t  begH;
  uint8_t  begM;
  uint8_t  endH;
  uint8_t  endM;
  uint32_t aCol;
  uint32_t bCol;
  uint32_t cCol;
  uint32_t dCol;
} Period;

#define MAX_PERIODS 9
typedef struct _BellSched
{
  uint8_t NumPeriods;
  Period Periods[MAX_PERIODS];
} BellSched;

typedef struct _SingleDay
{
  //bool        valid;
  uint16_t    Y;
  uint8_t     M;
  uint8_t     D;
  BellSched*  dayType;
  uint8_t     dayLetter;
} SingleDay;

const /*PROGMEM*/ BellSched NormalDay[]=
{
  9,
  { 8, 0, 8,50, OUTOFCLUSTER, RED,          OUTOFCLUSTER, YELLOW},
  { 8,52, 9,40, RED,          YELLOW,       YELLOW,       OUTOFCLUSTER},
  { 9,42, 9,52, ASPIRE,       ASPIRE,       ASPIRE,       ASPIRE},
  { 9,54,10,42, YELLOW,       OUTOFCLUSTER, RED,          RED},
  {10,44,11,32, GREEN,        OUTOFCLUSTER, BLUE,         GREEN},
  {11,34,12,23, OUTOFCLUSTER, BLUE,         GREEN,        PURPLE},
  {12,25,12,46, LUNCH,        LUNCH,        LUNCH,        LUNCH},
  {12,48,13,36, BLUE,         PURPLE,       PURPLE,       OUTOFCLUSTER},
  {13,38,14,26, PURPLE,       GREEN,        OUTOFCLUSTER, BLUE},
};

const /*PROGMEM*/ BellSched ER1115[]=
{
  7,
  { 8, 0, 8,28, OUTOFCLUSTER, RED,          OUTOFCLUSTER, YELLOW},
  { 8,30, 8,56, RED,          YELLOW,       YELLOW,       OUTOFCLUSTER},
  { 8,58, 9,24, YELLOW,       OUTOFCLUSTER, RED,          RED},
  { 9,26, 9,52, GREEN,        OUTOFCLUSTER, BLUE,         GREEN},
  { 9,54,10,20, OUTOFCLUSTER, BLUE,         GREEN,        PURPLE},
  {10,22,10,48, BLUE,         PURPLE,       PURPLE,       OUTOFCLUSTER},
  {10,50,11,15, PURPLE,       GREEN,        OUTOFCLUSTER, BLUE},
};

const /*PROGMEM*/ BellSched ER1200[]=
{
  7,
  { 8, 0, 8,33, OUTOFCLUSTER, RED,          OUTOFCLUSTER, YELLOW},
  { 8,35, 9, 8, RED,          YELLOW,       YELLOW,       OUTOFCLUSTER},
  { 9,10, 9,43, YELLOW,       OUTOFCLUSTER, RED,          RED},
  { 9,45,10,18, GREEN,        OUTOFCLUSTER, BLUE,         GREEN},
  {10,20,10,52, OUTOFCLUSTER, BLUE,         GREEN,        PURPLE},
  {10,54,11,26, BLUE,         PURPLE,       PURPLE,       OUTOFCLUSTER},
  {11,28,12,00, PURPLE,       GREEN,        OUTOFCLUSTER, BLUE},
};

const /*PROGMEM*/ BellSched ER1300[]=
{
  8,
  { 8, 0, 8,37, OUTOFCLUSTER, RED,          OUTOFCLUSTER, YELLOW},
  { 8,39, 9,16, RED,          YELLOW,       YELLOW,       OUTOFCLUSTER},
  { 9,18, 9,55, YELLOW,       OUTOFCLUSTER, RED,          RED},
  { 9,57,10,34, GREEN,        OUTOFCLUSTER, BLUE,         GREEN},
  {10,36,11,12, BLUE,         PURPLE,       PURPLE,       OUTOFCLUSTER},
  {11,14,12, 0, OUTOFCLUSTER, BLUE,         GREEN,        PURPLE},
  {12, 2,12,22, LUNCH,        LUNCH,        LUNCH,        LUNCH},
  {12,24,13, 0, PURPLE,       GREEN,        OUTOFCLUSTER, BLUE},
};

// days not present are either weekend days or holidays
const /*PROGMEM*/ SingleDay TheCalendar[]=
{
  {2018,9,4,NormalDay,'A'},
  {2018,9,5,NormalDay,'B'},
  {2018,9,6,NormalDay,'C'},
  {2018,9,7,NormalDay,'D'},
  {2018,9,11,NormalDay,'A'},
  {2018,9,12,NormalDay,'B'},
  {2018,9,13,NormalDay,'C'},
  {2018,9,14,NormalDay,'D'},
  {2018,9,17,NormalDay,'A'},
  {2018,9,18,ER1300,'B'},
  {2018,9,20,NormalDay,'C'},
  {2018,9,21,NormalDay,'D'},
  {2018,9,24,NormalDay,'A'},
  {2018,9,25,NormalDay,'B'},
  {2018,9,26,NormalDay,'C'},
  {2018,9,27,NormalDay,'D'},
  {2018,9,28,NormalDay,'A'},
  {2018,10,1,NormalDay,'B'},
  {2018,10,2,NormalDay,'C'},
  {2018,10,3,NormalDay,'D'},
  {2018,10,4,NormalDay,'A'},
  {2018,10,5,NormalDay,'B'},
  {2018,10,9,NormalDay,'C'},
  {2018,10,10,NormalDay,'D'},
  {2018,10,11,NormalDay,'A'},
  {2018,10,12,NormalDay,'B'},
  {2018,10,15,NormalDay,'C'},
  {2018,10,16,NormalDay,'D'},
  {2018,10,17,NormalDay,'A'},
  {2018,10,18,NormalDay,'B'},
  {2018,10,19,NormalDay,'C'},
  {2018,10,22,NormalDay,'D'},
  {2018,10,23,ER1300,'A'},
  {2018,10,24,NormalDay,'B'},
  {2018,10,25,NormalDay,'C'},
  {2018,10,26,NormalDay,'D'},
  {2018,10,29,NormalDay,'A'},
  {2018,10,30,NormalDay,'B'},
  {2018,10,31,NormalDay,'C'},
  {2018,11,1,ER1115,'D'},
  {2018,11,2,NormalDay,'A'},
  {2018,11,5,NormalDay,'B'},
  {2018,11,7,NormalDay,'C'},
  {2018,11,8,NormalDay,'D'},
  {2018,11,9,NormalDay,'A'},
  {2018,11,13,NormalDay,'B'},
  {2018,11,14,NormalDay,'C'},
  {2018,11,15,NormalDay,'D'},
  {2018,11,16,NormalDay,'A'},
  {2018,11,19,NormalDay,'B'},
  {2018,11,20,NormalDay,'C'},
  {2018,11,21,ER1200,'D'},
  {2018,11,26,NormalDay,'A'},
  {2018,11,27,NormalDay,'B'},
  {2018,11,28,NormalDay,'C'},
  {2018,11,29,NormalDay,'D'},
  {2018,11,30,NormalDay,'A'},
  {2018,12,3,NormalDay,'B'},
  {2018,12,4,NormalDay,'C'},
  {2018,12,5,NormalDay,'D'},
  {2018,12,6,NormalDay,'A'},
  {2018,12,7,NormalDay,'B'},
  {2018,12,10,NormalDay,'C'},
  {2018,12,11,NormalDay,'D'},
  {2018,12,12,NormalDay,'A'},
  {2018,12,13,NormalDay,'B'},
  {2018,12,14,NormalDay,'C'},
  {2018,12,17,NormalDay,'D'},
  {2018,12,18,ER1300,'A'},
  {2018,12,19,NormalDay,'B'},
  {2018,12,20,NormalDay,'C'},
  {2018,12,21,ER1200,'D'},
  {2019,1,2,NormalDay,'A'},
  {2019,1,3,NormalDay,'B'},
  {2019,1,4,NormalDay,'C'},
  {2019,1,7,NormalDay,'D'},
  {2019,1,8,NormalDay,'A'},
  {2019,1,9,NormalDay,'B'},
  {2019,1,10,NormalDay,'C'},
  {2019,1,11,NormalDay,'D'},
  {2019,1,14,NormalDay,'A'},
  {2019,1,15,ER1115,'B'},
  {2019,1,16,NormalDay,'C'},
  {2019,1,17,NormalDay,'D'},
  {2019,1,18,NormalDay,'A'},
  {2019,1,22,NormalDay,'B'},
  {2019,1,23,NormalDay,'C'},
  {2019,1,24,NormalDay,'D'},
  {2019,1,25,NormalDay,'A'},
  {2019,1,28,NormalDay,'B'},
  {2019,1,29,NormalDay,'C'},
  {2019,1,30,NormalDay,'D'},
  {2019,1,31,NormalDay,'A'},
  {2019,2,1,NormalDay,'B'},
  {2019,2,4,NormalDay,'C'},
  {2019,2,5,NormalDay,'D'},
  {2019,2,6,NormalDay,'A'},
  {2019,2,7,NormalDay,'B'},
  {2019,2,8,NormalDay,'C'},
  {2019,2,11,NormalDay,'D'},
  {2019,2,12,NormalDay,'A'},
  {2019,2,13,NormalDay,'B'},
  {2019,2,14,NormalDay,'C'},
  {2019,2,15,NormalDay,'D'},
  {2019,2,25,NormalDay,'A'},
  {2019,2,26,ER1300,'B'},
  {2019,2,27,NormalDay,'C'},
  {2019,2,28,NormalDay,'D'},
  {2019,3,1,NormalDay,'A'},
  {2019,3,4,NormalDay,'B'},
  {2019,3,5,NormalDay,'C'},
  {2019,3,6,NormalDay,'D'},
  {2019,3,7,NormalDay,'A'},
  {2019,3,8,NormalDay,'B'},
  {2019,3,11,NormalDay,'C'},
  {2019,3,12,NormalDay,'D'},
  {2019,3,13,NormalDay,'A'},
  {2019,3,14,NormalDay,'B'},
  {2019,3,15,NormalDay,'C'},
  {2019,3,18,NormalDay,'D'},
  {2019,3,19,ER1300,'A'},
  {2019,3,20,NormalDay,'B'},
  {2019,3,21,NormalDay,'C'},
  {2019,3,22,NormalDay,'D'},
  {2019,3,25,NormalDay,'A'},
  {2019,3,26,NormalDay,'B'},
  {2019,3,27,NormalDay,'C'},
  {2019,3,28,NormalDay,'D'},
  {2019,3,29,NormalDay,'A'},
  {2019,4,1,NormalDay,'B'},
  {2019,4,2,NormalDay,'C'},
  {2019,4,3,NormalDay,'D'},
  {2019,4,4,NormalDay,'A'},
  {2019,4,5,NormalDay,'B'},
  {2019,4,8,NormalDay,'C'},
  {2019,4,9,NormalDay,'D'},
  {2019,4,10,NormalDay,'A'},
  {2019,4,11,NormalDay,'B'},
  {2019,4,12,NormalDay,'C'},
  {2019,4,22,NormalDay,'D'},
  {2019,4,23,ER1300,'A'},
  {2019,4,24,NormalDay,'B'},
  {2019,4,25,NormalDay,'C'},
  {2019,4,26,NormalDay,'D'},
  {2019,4,29,NormalDay,'A'},
  {2019,4,30,NormalDay,'B'},
  {2019,5,1,NormalDay,'C'},
  {2019,5,2,NormalDay,'D'},
  {2019,5,3,NormalDay,'A'},
  {2019,5,6,NormalDay,'B'},
  {2019,5,7,NormalDay,'C'},
  {2019,5,8,NormalDay,'D'},
  {2019,5,9,NormalDay,'A'},
  {2019,5,10,NormalDay,'B'},
  {2019,5,13,NormalDay,'C'},
  {2019,5,14,NormalDay,'D'},
  {2019,5,15,NormalDay,'A'},
  {2019,5,16,NormalDay,'B'},
  {2019,5,17,NormalDay,'C'},
  {2019,5,20,NormalDay,'D'},
  {2019,5,21,ER1300,'A'},
  {2019,5,22,NormalDay,'B'},
  {2019,5,23,NormalDay,'C'},
  {2019,5,24,NormalDay,'D'},
  {2019,5,28,NormalDay,'A'},
  {2019,5,29,NormalDay,'B'},
  {2019,5,30,NormalDay,'C'},
  {2019,5,31,NormalDay,'D'},
  {2019,6,3,NormalDay,'A'},
  {2019,6,4,NormalDay,'B'},
  {2019,6,5,NormalDay,'C'},
  {2019,6,6,NormalDay,'D'},
  {2019,6,7,NormalDay,'A'},
  {2019,6,10,NormalDay,'B'},
  {2019,6,11,NormalDay,'C'},
  {2019,6,12,NormalDay,'D'},
  {2019,6,13,NormalDay,'A'},
  {2019,6,14,NormalDay,'B'},
  {2019,6,17,NormalDay,'C'},
  {2019,6,18,NormalDay,'D'},
  {2019,6,19,NormalDay,'A'},
  {2019,6,20,NormalDay,'B'},
  {2019,6,21,NormalDay,'C'},
  {2019,6,24,NormalDay,'D'},
  {2019,6,25,NormalDay,'A'},
  {2019,6,26,NormalDay,'B'},
  {2019,6,27,NormalDay,'C'},
  {2019,6,28,NormalDay,'D'},
};

const uint8_t DayCount = sizeof(TheCalendar)/sizeof(SingleDay);
SingleDay *Today = 0;

/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
// CUSTOMIZE THIS STUFF
/////////////////////////////////////////////////////////


// number of minutes before end of class when countdown clock is triggered
uint8_t countdownM = 6;
uint8_t secBetweenFlashes = 4;

/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
// THE GUTS
/////////////////////////////////////////////////////////
void setup()
{
  #ifdef DEBUG
  Serial.begin(57600);
  #endif
  /*
   * For testing purposes, you can set the clock to custom values, e.g.:
   * initChronoDot(year, month, day, hour, minute, seconds);
   * Otherwise, the clock automatically sets itself to your computer's
   * time with the function initChronoDot();
  */
  // initChronoDot(2016, 11, 7, 15, 59, 50);
  initChronoDot();
  strip.begin();
  strip.show();
  delay(3000);
}

void loop()
{
  now = RTC.now();
  if(now.minute()!=currentMinute)
  {
    // let's figure out all the interesting state info on the new minute boundary
    if(now.day()!=currentDay) DoNewDayStuff();
    else DoNewMinuteStuff();
  }
  displayClock();
}

void DoNewDayStuff()
{
  currentPeriod = 0;
  currentDay = now.day();
  if ( isWeekend() )
  {
    dayType = dtWeekend;
    clockType = ctWeekend;
    Today = 0;
  }
  else
  {
    Today = isSchoolDay();
    if ( 0 != Today )
    {
      dayType = dtSchoolDay;
      clockType = ctBeforeSchool;
    }
    else
    {
      dayType = dtHoliday;
      clockType = ctHoliday;
    }
  }
}

void DoNewMinuteStuff()
{
  currentMinute = now.minute();
  if ( dayType == dtWeekend || dayType == dtHoliday || clockType == ctAfterSchool );
  else
  {
    //check against bellschedule, set pointer to current period, and set clock type
    BellSched *bs = Today->dayType;
    if ( clockType == ctBeforeSchool )
    {
      // see if we've made it to first period
      Period *p = &bs->Periods[0];
      if ( now.hour() == p->begH && now.minute() == p->begM )
      {
        currentPeriod = 0;
        clockType = ctDuringClass;
      }
    }
    else
    {
      Period *p = &bs->Periods[currentPeriod];
      if ( currentPeriod < bs->NumPeriods-1 )
      {
        Period *np = &bs->Periods[currentPeriod+1];
        // consider we may have started the next period
        if ( isAfterTime(now.hour(), now.minute(), np->begH, np->begM) )
        {
          clockType = ctDuringClass;
          currentPeriod++;
          return;
        }
      }
      if ( timeDiff(p->endH, p->endM, now.hour(), now.minute())  < countdownM )
      {
        clockType = ctEndFlash;
      }
      else if ( isAfterTime(now.hour(), now.minute(), p->endH, p->endM) )
      {
        if ( currentPeriod < bs->NumPeriods-1 )
        {
          clockType = ctPassing;
        }
        else clockType = ctAfterSchool;
      }
    }
  }
}

/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
// CUSTOMIZE DISPLAY FUNCTIONALITY
/////////////////////////////////////////////////////////

void displayClock()
{
  switch (clockType)
  {
    case ctWeekend:
    colorClock(Wheel(0));
    break;
    case ctHoliday:
    colorClock(Wheel(20));
    break;
    case ctBeforeSchool:
    pulseClock(Wheel(40), 5);
    break;
    case ctAfterSchool:
    colorClock(Wheel(100));
    break;
    case ctEndFlash:
    countdownClock();
    break;
    case ctDuringClass:
    gradientClock();
    break;
    case ctPassing:
    mardiGrasClock();
    break;
  }
}


/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
// CHECK
/////////////////////////////////////////////////////////
// returns true if the first number is before the
boolean isBeforeTime(uint8_t h0, uint8_t m0, uint8_t h1, uint8_t m1)
{
  if (timeDiff(h0, m0, h1, m1) < 0) return true;
  else return false;
}

boolean isAfterTime(uint8_t h0, uint8_t m0, uint8_t h1, uint8_t m1)
{
  if (timeDiff(h0, m0, h1, m1) >= 0) return true;
  else return false;
}

// returns the difference in minutes
// later time first
int timeDiff(uint8_t h0, uint8_t m0, uint8_t h1, uint8_t m1)
{
  uint16_t t0 = h0 * 60 + m0;
  uint16_t t1 = h1 * 60 + m1;
  return t0 - t1;
}

SingleDay *isSchoolDay()
{
  if (isWeekend()) return 0;
  for (uint8_t i=0; i<DayCount; i++)
  {
    // loop thru calendar until we've either found the current day, or gone past
    SingleDay *d = &TheCalendar[i];
    if ( d->D == now.day() && d->M == now.month() && d->Y == now.year() ) return d;
    else if ( d->Y > now.year() ) break;
    else if ( d->Y == now.year() && d->M > now.month() ) break;
    else if (d->Y == now.year() && d->M == now.month() && d->D > now.day()) break;
  }
  return 0;
}

boolean isWeekend()
{
  // 0 = Sunday, 1 = Monday, ...., 6 = Saturday
  if (now.dayOfTheWeek() == 0 || now.dayOfTheWeek() == 6) return true;
  return false;
}

boolean isBetweenTime(uint8_t h0, uint8_t m0, uint8_t h1, uint8_t m1)
{
  DateTime startTime (now.year(), now.month(), now.day(), h0, m0, 0);
  DateTime endTime (now.year(), now.month(), now.day(), h1, m1, 0);
  return (now.unixtime() >= startTime.unixtime() && now.unixtime() < endTime.unixtime());
}

/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
// DISPLAY
/////////////////////////////////////////////////////////

void displayColon(uint32_t c)
{
  if (now.second() % 2 == 0)
  {
    strip.setPixelColor(21, c);
    strip.setPixelColor(22, c);
  }
  else
  {
    strip.setPixelColor(21, 0);
    strip.setPixelColor(22, 0);
  }
}

void countdownClock()
{
  Period *p=&Today->dayType->Periods[currentPeriod];
  uint8_t h = p->endH;
  uint8_t m = p->endM;
  DateTime endTime(now.year(), now.month(), now.day(), h, m, 0);
  uint8_t minLeft = (endTime.unixtime() - now.unixtime()) / 60;
  uint8_t secLeft = (endTime.unixtime() - now.unixtime()) - minLeft * 60;
  if (minLeft == 0) displayHour(minLeft, 0);
  else displayHour(minLeft, Wheel(190));
  displayMinute(secLeft, Wheel(190));
  displayLetter(getLetter());
  displayColon(Wheel(190));
  strip.show();
}

void colorClock(int c)
{
  displayHour(now.hour(), Wheel(c));
  displayMinute(now.minute(), Wheel(c));
  displayLetter(getLetter());
  displayColon(Wheel(c));
  strip.show();
}

void rainbowClock(int delayTime)
{
  for (int j = 0; j < 256; j++)
  {
    displayHour(now.hour(), Wheel(j));
    displayMinute(now.minute(), Wheel(j));
    displayLetter(getLetter());
    displayColon(Wheel(j));
    strip.show();
    unsigned long t = millis();
    while (millis() - t < delayTime)
    {
      displayColon(Wheel(j));
      strip.show();
    }
  }
}

void mardiGrasClock()
{
  displayHour(now.hour(), Wheel(200));
  displayMinute(now.minute(), Wheel(80));
  displayLetter(getLetter());
  displayColon(Wheel(50));
  strip.show();
}

void randoClock(int delayTime)
{
  displayHour(now.hour(), Wheel(random(0, 255)));
  displayMinute(now.minute(), Wheel(random(0, 255)));
  displayLetter(getLetter());
  displayColon(Wheel(random(0, 255)));
  unsigned long t = millis();
  while (millis() - t < delayTime) displayColon(Wheel(random(0, 255)));
}

void pulseClock(uint32_t col, int delayTime)
{
  for (int j = 255; j >= 0; j--)
  {
    displayHour(now.hour(), col);
    displayMinute(now.minute(), col);
    displayLetter(getLetter());
    displayColon(col);
    strip.setBrightness(j);
    strip.show();
    unsigned long t = millis();
    while (millis() - t < delayTime)
    {
      displayColon(col);
      strip.show();
    }
  }
  for (int j = 0; j < 256; j++)
  {
    displayHour(now.hour(), col);
    displayMinute(now.minute(), col);
    displayLetter(getLetter());
    displayColon(col);
    strip.setBrightness(j);
    strip.show();
    unsigned long t = millis();
    while (millis() - t < delayTime)
    {
      displayColon(col);
      strip.show();
    }
  }
}

void gradientClock()
{
  uint32_t c = Wheel(getGradientColor(now.hour(), now.minute()));
  displayHour(now.hour(), c);
  displayMinute(now.minute(), c);
  displayColon(c);
  displayLetter(getLetter());
  strip.show();
}

int getGradientColor(uint8_t h, uint8_t m)
{
  // DateTime (year, month, day, hour, min, sec);
  Period  *p=&Today->dayType->Periods[currentPeriod];
  uint8_t h0 = p->begH;
  uint8_t m0 = p->begM;
  uint8_t h1 = p->endH;
  uint8_t m1 = p->endM;
  DateTime startTime(now.year(), now.month(), now.day(), h0, m0, 0);
  DateTime endTime(now.year(), now.month(), now.day(), h1, m1, 0);
  #ifdef DEBUG
  Serial.print("Gradient Clock: ");
  Serial.print(map(now.unixtime(), startTime.unixtime(), endTime.unixtime(), 0, 100));
  Serial.println("% through period");
  #endif
  return map(now.unixtime(), startTime.unixtime(), endTime.unixtime(), 80, 0);
}

void displayHour(uint8_t h, uint32_t col)
{
  if (h == 0) h = 12;
  else if (h > 12) h -= 12;
  uint8_t firstDigit = h / 10;
  uint8_t secondDigit = h % 10;
  // TODO - this only works for non-military time
  // firstDigit is either off or all on (0 or 1)
  if (firstDigit > 0)
  {
    strip.setPixelColor(30, col);
    strip.setPixelColor(31, col);
  }
  else
  {
    strip.setPixelColor(30, 0);
    strip.setPixelColor(31, 0);
  }
  // secondDigit
  for (int i = 0; i < 7; i++)
  {
    if (numbers[secondDigit] & (1 << 7 - i)) strip.setPixelColor(i + (7 * 3 + 2), col);
    else strip.setPixelColor(i + (7 * 3 + 2), 0);
  }
  if (now.minute() / 10 == 2) strip.setPixelColor(14, 0);
}

void displayMinute(uint8_t m, uint32_t col)
{
  uint8_t firstDigit = m / 10;
  uint8_t secondDigit = m % 10;

  // first digit (first from left to right)
  for (int i = 0; i < 7; i++)
  {
    if (numbers[firstDigit] & (1 << 7 - i)) strip.setPixelColor(i + (7 * 2), col);
    else
    {
      strip.setPixelColor(i + (7 * 2), 0);
    }
  }
  // second digit (least significant)
  for (int i = 0; i < 7; i++)
  {
    if (numbers[secondDigit] & (1 << 7 - i))
    {
      strip.setPixelColor(i + 7, col);
    }
    else
    {
      strip.setPixelColor(i + 7, 0);
    }
  }
}

uint32_t getLetterColor()
{
  Period *p = &Today->dayType->Periods[currentPeriod];
  uint8_t dl=Today->dayLetter;
  switch(dl)
  {
    case 'A':
    return p->aCol;
    case 'B':
    return p->bCol;
    case 'C':
    return p->cCol;
    case 'D':
    return p->dCol;
    default:
    return 0;
  }
}

void displayLetter(uint8_t letter)
{
  letter -= 65; // offset for 'A'
  uint32_t col = getLetterColor();
  if (letter < 8)
  {
    for (int i = 0; i < 7; i++)
    {
      if (letters[letter] & (1 << 7 - i)) strip.setPixelColor(i, col);
      else strip.setPixelColor(i, 0);
    }
  }
  else
  {
    for (int i = 0; i < 7; i++)
    {
      strip.setPixelColor(i, 0);
    }
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
// credit - Adafruit
uint32_t Wheel(byte WheelPos)
{
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  else if (WheelPos < 170)
  {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  else
  {
    WheelPos -= 170;
    return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  }
}

/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
// GET/SET
/////////////////////////////////////////////////////////
// credit: Adafruit
// Use this function to automatically set an initial DateTime
// using time from the computer/ compiler
void initChronoDot()
{
  // Instantiate the RTC
  Wire.begin();
  RTC.begin();
  // Check if the RTC is running.
  #ifdef DEBUG
  if (! RTC.isrunning()) Serial.println("RTC is NOT running");
  #endif
  // This section grabs the current datetime and compares it to
  // the compilation time.  If necessary, the RTC is updated.
  now = RTC.now();
  DateTime compiled = DateTime(__DATE__, __TIME__);
  if (now.unixtime() < compiled.unixtime())
  {
    #ifdef DEBUG
    Serial.println("RTC is older than compile time! Updating");
    #endif
    RTC.adjust(DateTime(__DATE__, __TIME__));
  }
  now = RTC.now();
  #ifdef DEBUG
  Serial.println("Setup complete.");
  #endif
}

// Use this function to set a specific initial DateTime
// useful for debugging
void initChronoDot(int y, int mon, int d, int h, int minu, int s)
{
  // Instantiate the RTC
  Wire.begin();
  RTC.begin();
  // Check if the RTC is running.
  if (! RTC.isrunning())
  {
     #ifdef DEBUG
     Serial.println("RTC is NOT running");
     #endif
  }
  now = RTC.now();
  RTC.adjust(DateTime(y, mon, d, h, minu, s));
  now = RTC.now();
  #ifdef DEBUG
  Serial.println("Setup complete.");
  #endif
}

uint8_t getLetter()
{
  if ( dayType == dtWeekend || dayType == dtHoliday || clockType == ctBeforeSchool || clockType == ctAfterSchool ) return 9;
  else return Today->dayLetter;
}

void printClock()
{
  #ifdef DEBUG
  Serial.print(now.hour());
  Serial.print(":");
  Serial.print(now.minute());
  Serial.print(":");
  Serial.println(now.second());
  #endif
}
