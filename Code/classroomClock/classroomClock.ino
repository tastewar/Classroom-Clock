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
uint8_t currentPeriod = 0;
uint8_t lastHour = 0;
uint8_t timeBlockIndex = 0;

DateTime lastFlash;    // for countdown "flash"
boolean flashOn = false;
long lastSpeedTest = 0;

const PROGMEM uint8_t numbers[] = {
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
const PROGMEM uint8_t letters[] = {
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

/*#define MAX_DAYS  180
typedef struct _Calendar
{
  SingleDay Days[MAX_DAYS];
} Calendar;*/

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

/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
// CUSTOMIZE THIS STUFF
/////////////////////////////////////////////////////////

/*
 * Set the functionality of the extra digit
 * show nothing in the extra digit is mode 0
 * show rotating block is mode 1
 * show period number is mode 2
*/
int extraDigitMode = 1;

// setup for a rotating block schedule
uint8_t currentBlock = D_BLOCK;

// this number should match the number of entries in schedule[]
const uint8_t numTimeBlocks = 9;

const PROGMEM uint8_t schedule[numTimeBlocks][5] =
{
  // use 24 hour clock numbers even though this clock is a 12 hour clock
  // {starting hour, start min, end hour, end min}
  {8, 0, 8, 50, ACADEMIC},       // 0 - Period 1:
  {8, 52, 9, 40, ACADEMIC},      // 1 - Period 2:
  {9, 42, 9, 52, ASSEMBLY},    // 2 - Aspire:
  {9, 54, 10, 42, ACADEMIC},    // 3 - Period 3:
  {10, 44, 11, 32, ACADEMIC},    // 4 - Period 4:
  {11, 34, 12, 23, ACADEMIC},    // 5 - Period 5: (2/3)
  {12, 25, 12, 46, LUNCH},      // 6 - Lunch: 
  {12, 48, 13, 36, ACADEMIC},   // 7 - Period 6:
  {13, 38, 14, 26, ACADEMIC}     // 8 - Period 7:
};

// this number should match the number of entries in holidays[]
const uint8_t numHolidays = 3;
const PROGMEM uint8_t holidays[numHolidays][3] =
{
  {2016, 1, 10},
  {2016, 2, 20},
  {2016, 3, 30}
};

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
  setSchedule();
  delay(3000);
}

void loop()
{
  now = RTC.now();
  if (isEndOfDay()) nextDay();
  if (isSchoolDay()) checkBlock();
  displayClock();
}

/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
// CUSTOMIZE DISPLAY FUNCTIONALITY
/////////////////////////////////////////////////////////

void displayClock()
{
  if (isWeekend()) colorClock(Wheel(0));
  else if (!isSchoolDay()) colorClock(Wheel(20));
  else if (isBeforeSchool()) pulseClock(Wheel(40), 5);
  else if (isAfterSchool()) colorClock(Wheel(100));
  else if (isEndFlash()) countdownClock();
  else if (isLunch()) birthdayClock(300);
  else if (isAssembly()) rainbowClock(5);
  else if (isDuringClass()) gradientClock();
  else
  {
    //between classes
    mardiGrasClock();
  }
}


/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
// CHECK
/////////////////////////////////////////////////////////
void checkBlock()
{
  if (timeBlockIndex < numTimeBlocks)
  {
    uint8_t h = schedule[timeBlockIndex][2];
    uint8_t m = schedule[timeBlockIndex][3];
    if (isAfterTime(now.hour(), now.minute(), h, m))
    {
      if (schedule[timeBlockIndex][4] == ACADEMIC)
      {
        currentPeriod++;
        currentBlock++;
        if (currentBlock == 8) currentBlock = 0;
        #ifdef DEBUG
        Serial.print("Academic period changed to: ");
        Serial.print(currentPeriod);
        Serial.print(" & block changed to: ");
        Serial.println(currentBlock);
        #endif
      }
      timeBlockIndex++;
      #ifdef DEBUG
      Serial.print("Time block index: ");
      Serial.println(timeBlockIndex);
      #endif
    }
  }
}

int getCurrentPeriod()
{
  int period = -1;
  if (isBeforeSchool() || isAfterSchool()) period = 0;
  // check if it's before the end of an academic period
  // if it's not during an academic period, find the next academic period
  for (int i = 0; i < numTimeBlocks; i++ )
  {
    if (schedule[i][4] == ACADEMIC) period++;
    if (isBeforeTime(now.hour(), now.minute(), schedule[i][2], schedule[i][3])) break;
  }
  return period;
}

int getCurrentTimeBlock()
{
  int tb = 0;
  if (isBeforeSchool() || isAfterSchool()) tb = 0;
  // check if it's before the end of an academic period
  // if it's not during an academic period, find the next academic period
  for (int i = 0; i < numTimeBlocks; i++ )
  {
    if (isBeforeTime(now.hour(), now.minute(), schedule[i][2], schedule[i][3])) return i;
  }
  return tb;
}

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

boolean isEndFlash()
{
  uint8_t h;
  uint8_t m;
  if (isDuringTimeBlocks())
  {
    h = schedule[timeBlockIndex][2];
    m = schedule[timeBlockIndex][3];
  }
  else if (isBetweenTimeBlocks())
  {
    h = schedule[timeBlockIndex][0];
    m = schedule[timeBlockIndex][1];
  }
  else return false;
  if (timeDiff(h, m, now.hour(), now.minute())  < countdownM)
  {
    #ifdef DEBUG
    Serial.print("End Flash: ");
    Serial.print(timeDiff( h, m, now.hour(), now.minute()));
    Serial.println(" minutes remaining");
    #endif
    if (now.unixtime() - lastFlash.unixtime() > secBetweenFlashes)
    {
      lastFlash = now;
      flashOn = !flashOn;
    }
    return flashOn;
  }
  return false;
}

boolean isSchoolDay()
{
  if (isWeekend()) return false;
  else if (isHoliday()) return false;
  return true;
}

boolean isWeekend()
{
  // 0 = Sunday, 1 = Monday, ...., 6 = Saturday
  if (now.dayOfTheWeek() == 0 || now.dayOfTheWeek() == 6) return true;
  return false;
}

boolean isHoliday()
{
  for (int i = 0; i < numHolidays; i++)
  {
    if(now.year() == holidays[i][0] && now.month() == holidays[i][1] && now.day() == holidays[i][2]) return true;
  }
  return false;
}

boolean isBetweenTime(uint8_t h0, uint8_t m0, uint8_t h1, uint8_t m1)
{
  DateTime startTime (now.year(), now.month(), now.day(), h0, m0, 0);
  DateTime endTime (now.year(), now.month(), now.day(), h1, m1, 0);
  return (now.unixtime() >= startTime.unixtime() && now.unixtime() < endTime.unixtime());
}

boolean isLunch()
{
  if (schedule[timeBlockIndex][4] == LUNCH)
  {
    #ifdef DEBUG
    Serial.println("Lunch!");
    #endif
    return true;
  }
  else return false;
}

boolean isAssembly()
{
  if (schedule[timeBlockIndex][4] == ASSEMBLY)
  {
    #ifdef DEBUG
    Serial.println("Assembly!");
    #endif
    return true;
  }
  return false;
}

boolean isAfterSchool()
{
  // You may need to edit these values if school ends with a homeroom or
  // otherBlock[] that isn't a classperiod[]
  if (now.hour() >= schedule[numTimeBlocks - 1][2] && now.minute() >=  schedule[numTimeBlocks - 1][3])
  {
    #ifdef DEBUG
    Serial.println("After School!");
    #endif
    return true;
  }
  return false;
}

boolean isBeforeSchool()
{
  // You may need to edit these values if school begins with a homeroom or
  // otherBlock[] that isn't a classperiod[]
  if (now.hour() <= schedule[0][0] && now.minute() < schedule[0][1])
  {
    #ifdef DEBUG
    Serial.println("Before School!");
    #endif
    return true;
  }
  return false;
}

boolean isEndOfDay()
{
  boolean changed = false;
  if (now.hour() == 0 && lastHour == 23) changed = true;
  lastHour = now.hour();
  return changed;
}

boolean isBetweenTimeBlocks()
{
  return isDuringSchoolDay() && !isDuringTimeBlocks();
}

boolean isHourChange()
{
  if (now.minute() == 0 && now.second() < 15) return true;
  return false;
}

boolean isEnd()
{
  uint8_t h = schedule[currentPeriod][2];
  uint8_t m = schedule[currentPeriod][3];
  DateTime endTime (now.year(), now.month(), now.day(), h, m, 0);
  if (endTime.unixtime() - now.unixtime()  < 7 * 60) return true;
  else return false;
}

boolean isDuringSchoolDay()
{
  return isSchoolDay() && !isAfterSchool() && !isBeforeSchool();
}

boolean isDuringClass()
{
  for (int i = 0; i < numTimeBlocks; i++ )
  {
    if (isBetweenTime(schedule[i][0], schedule[i][1], schedule[i][2], schedule[i][3]))
    {
      if (schedule[i][4] == ACADEMIC)
      {
        #ifdef DEBUG
        Serial.println("During class!");
        #endif
        return true;
      }
    }
  }
  return false;
}

boolean isDuringTimeBlocks()
{
  for (int i = 0; i < numTimeBlocks; i++)
  {
    if (isBetweenTime(schedule[i][0], schedule[i][1], schedule[i][2], schedule[i][3]))
    {
      #ifdef DEBUG
      Serial.println("During a time block!");
      #endif
      return true;
    }
  }
  return false;
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
  uint8_t h;
  uint8_t m;
  if (isDuringTimeBlocks())
  {
    h = schedule[timeBlockIndex][2];
    m = schedule[timeBlockIndex][3];
  }
  else if (isBetweenTimeBlocks())
  {
    h = schedule[timeBlockIndex][0];
    m = schedule[timeBlockIndex][1];
  }
  DateTime endTime(now.year(), now.month(), now.day(), h, m, 0);
  uint8_t minLeft = (endTime.unixtime() - now.unixtime()) / 60;
  uint8_t secLeft = (endTime.unixtime() - now.unixtime()) - minLeft * 60;
  if (minLeft == 0) displayHour(minLeft, 0);
  else displayHour(minLeft, Wheel(190));
  displayMinute(secLeft, Wheel(190));
  displayLetter(getLetter(), Wheel(190));
  displayColon(Wheel(190));
  strip.show();
}

void colorClock(int c)
{
  displayHour(now.hour(), Wheel(c));
  displayMinute(now.minute(), Wheel(c));
  displayLetter(getLetter(), Wheel(c));
  displayColon(Wheel(c));
  strip.show();
}

void rainbowClock(int delayTime)
{
  for (int j = 0; j < 256; j++)
  {
    displayHour(now.hour(), Wheel(j));
    displayMinute(now.minute(), Wheel(j));
    displayLetter(getLetter(), Wheel(j));
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

void birthdayClock(int delayTime)
{
  displayHour(now.hour(), Wheel(random(0, 255)));
  displayMinute(now.minute(), Wheel(random(0, 255)));
  displayLetter(getLetter(), Wheel(random(0, 255)));
  displayColon(Wheel(random(0, 255)));
  unsigned long t = millis();
  strip.show();
  while (millis() - t < delayTime) displayColon(Wheel(random(0, 255)));
}

void xmasClock()
{
  displayHour(now.hour(), Wheel(0));
  displayMinute(now.minute(), Wheel(80));
  displayLetter(getLetter(), Wheel(0));
  displayColon(Wheel(0));
  strip.show();
}

void mardiGrasClock()
{
  displayHour(now.hour(), Wheel(200));
  displayMinute(now.minute(), Wheel(80));
  displayLetter(getLetter(), Wheel(50));
  displayColon(Wheel(50));
  strip.show();
}

void randoClock(int delayTime)
{
  displayHour(now.hour(), Wheel(random(0, 255)));
  displayMinute(now.minute(), Wheel(random(0, 255)));
  displayLetter(getLetter(), Wheel(random(0, 255)));
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
    displayLetter(getLetter(), col);
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
    displayLetter(getLetter(), col);
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
  displayLetter(getLetter(), c);
  strip.show();
}

int getGradientColor(uint8_t h, uint8_t m)
{
  // DateTime (year, month, day, hour, min, sec);
  uint8_t h0 = schedule[timeBlockIndex][0];
  uint8_t m0 = schedule[timeBlockIndex][1];
  uint8_t h1 = schedule[timeBlockIndex][2];
  uint8_t m1 = schedule[timeBlockIndex][3];
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

void displayLetter(uint8_t letter, uint32_t col)
{
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
  if (extraDigitMode == 0) return 9;
  else if (!isSchoolDay()) return 9;
  else if (isAfterSchool()) return 9;
  else if (extraDigitMode == 2) return currentPeriod + 1;
  else return currentBlock;
}

void setSchedule()
{
  currentPeriod = getCurrentPeriod();
  timeBlockIndex = getCurrentTimeBlock();
  #ifdef DEBUG
  Serial.print("Current (starting) period is set to: ");
  Serial.println(currentPeriod);
  Serial.print("Current (starting) time block index is set to: ");
  Serial.println(timeBlockIndex);
  #endif
}


void nextDay()
{
  if (isSchoolDay())
  {
    currentPeriod = 0;
    timeBlockIndex = 0;
  }
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
