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
#include <Adafruit_NeoPixel.h>    // https://github.com/adafruit/Adafruit_NeoPixel

#define NEOPIXEL_PIN 3
#define NUM_PIXELS 32
#define TODAY_IS_A_HOLIDAY 255
#define TODAY_IS_A_WEEKEND 254

//#define DEBUG
//#define DEBUG2
//#define DEBUG3

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_PIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);
RTC_DS1307 RTC;
DateTime now;
uint8_t currentMinute = 60;
uint8_t currentPeriod = 0;
uint8_t currentDay = 0;

////////////////////////////////////////////////////////
//// CLOCK ADJUSTMENTS
////////////////////////////////////////////////////////
////////////////////////////////////////////////////////

#define COMPILE_TIME  12
#define OTTOSON_CLOCK_CORRECTION -80

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

#define MAX_PERIODS 10
typedef struct _BellSched
{
  uint8_t NumPeriods;
  Period Periods[MAX_PERIODS];
} BellSched;

typedef struct _SingleDay
{
  uint16_t    Y;
  uint8_t     M;
  uint8_t     D;
  const BellSched*  dayType;
} SingleDay;

#include "Schedules201920.h"

#include "Calendar201920.h"

const uint8_t DayCount = sizeof(TheCalendar)/sizeof(SingleDay);
uint8_t Today = 255;

/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
// CUSTOMIZE THIS STUFF
/////////////////////////////////////////////////////////


// number of minutes before end of class when countdown clock is triggered
uint8_t countdownM = 5;
uint8_t secBetweenFlashes = 4;

/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
// THE GUTS
/////////////////////////////////////////////////////////
void setup()
{
  #ifdef DEBUG
  Serial.begin(9600);
  #endif
  /*
   * For testing purposes, you can set the clock to custom values, e.g.:
   * initChronoDot(year, month, day, hour, minute, seconds);
   * Otherwise, the clock automatically sets itself to your computer's
   * time with the function initChronoDot();
  */
  //initChronoDot(2019, 1, 15, 7, 56, 50);
  initChronoDot();
  now = RTC.now();
  DoNewDayStuff();
  DeterminePeriod();
  DoNewMinuteStuff();
  strip.begin();
  strip.show();
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

void DeterminePeriod()
{
  if ( Today == TODAY_IS_A_WEEKEND )
  {
    clockType = ctWeekend;
  }
  else if ( Today == TODAY_IS_A_HOLIDAY )
  {
    clockType = ctHoliday;
  }
  else
  {
    BellSched *bs=CalGetDayType(Today);
    uint8_t p,n=BSGetNumPeriods(bs);
  
    if (isAfterTime(now.hour(), now.minute(), BSGetEndHour(bs,n-1), BSGetEndMin(bs,n-1)))
    {
      currentPeriod=n;
      clockType = ctAfterSchool;
      return;
    }
    else for (p=0;p<n;p++)
    {
      if (!isAfterTime(now.hour(), now.minute(), BSGetBegHour(bs,p), BSGetBegMin(bs,p))) break;
    }
    if (p==0)
    {
      //before the first period
      currentPeriod=0;
      clockType=ctBeforeSchool;
      return;
    }
    else
    {
      currentPeriod = p-1;
      if (isAfterTime(now.hour(), now.minute(), BSGetEndHour(bs,currentPeriod), BSGetEndMin(bs,currentPeriod)))
      {
        clockType = ctPassing;
      }
      else clockType = ctDuringClass;
    }
  }  
}

void DoNewDayStuff()
{
  currentPeriod = 0;
  currentDay = now.day();
  if ( isWeekend() )
  {
    dayType = dtWeekend;
    clockType = ctWeekend;
    Today = TODAY_IS_A_WEEKEND;
  }
  else
  {
    Today = isSchoolDay();
    if ( TODAY_IS_A_HOLIDAY != Today )
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
    BellSched *bs = CalGetDayType(Today);
    if ( clockType == ctBeforeSchool )
    {
      // see if we've made it to first period
      if ( now.hour() == BSGetBegHour(bs,0) && now.minute() == BSGetBegMin(bs,0) )
      {
        currentPeriod = 0;
        clockType = ctDuringClass;
      }
    }
    else
    {
      if ( currentPeriod < BSGetNumPeriods(bs)-1 )
      {
        // consider we may have started the next period
        if ( isAfterTime(now.hour(), now.minute(), BSGetBegHour(bs,currentPeriod+1), BSGetBegMin(bs,currentPeriod+1)) )
        {
          clockType = ctDuringClass;
          currentPeriod++;
          return;
        }
      }
      uint16_t td=timeDiff(BSGetEndHour(bs,currentPeriod), BSGetEndMin(bs,currentPeriod), now.hour(), now.minute());
      if ( td > 0 && td <= countdownM )
      {
        clockType = ctEndFlash;
      }
      else if ( isAfterTime(now.hour(), now.minute(), BSGetEndHour(bs,currentPeriod), BSGetEndMin(bs,currentPeriod)) )
      {
        if ( currentPeriod < BSGetNumPeriods(bs)-1 )
        {
          clockType = ctPassing;
        }
        else clockType = ctAfterSchool;
      }
    }
  }
  #ifdef DEBUG
  printClock();
  Serial.print(F("Clock Type: "));
  Serial.print(clockType);
  Serial.print(F("\nDay Type: "));
  Serial.print(dayType);
  Serial.print(F("\nToday: "));
  Serial.print(Today);
  Serial.print(F(" Current Period: "));
  Serial.print(currentPeriod);
  Serial.print("\n");
  #endif 
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
    colorClock(0);
    break;
    case ctHoliday:
    colorClock(20);
    break;
    case ctBeforeSchool:
    pulseClock(Wheel(40), 5);
    break;
    case ctAfterSchool:
    colorClock(100);
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

uint8_t isSchoolDay()
{
  // returns index into TheCalendar array
  if (isWeekend()) return TODAY_IS_A_WEEKEND;
  for (uint8_t i=0; i<DayCount; i++)
  {
    uint16_t cy=CalGetYear(i);
    uint8_t cm=CalGetMonth(i);
    uint8_t cd=CalGetDay(i);

    #ifdef DEBUG2
    Serial.print(cy);
    Serial.print(cm);
    Serial.print(cd);
    Serial.print(" ");
    #endif
    
    // loop thru calendar until we've either found the current day, or gone past
    if ( cd == now.day() && cm == now.month() && cy == now.year() ) return i;
    else if ( cy > now.year() ) break;
    else if ( cy == now.year() && cm > now.month() ) break;
    else if ( cy == now.year() && cm == now.month() && cd > now.day()) break;
  }
  return TODAY_IS_A_HOLIDAY;
}

boolean isWeekend()
{
  // 0 = Sunday, 1 = Monday, ...., 6 = Saturday
  if (now.dayOfTheWeek() == 0 || now.dayOfTheWeek() == 6) return true;
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
  BellSched *bs=CalGetDayType(Today);
  Period *p=&bs->Periods[currentPeriod];
  uint8_t h = BSGetEndHour(bs,currentPeriod);
  uint8_t m = BSGetEndMin(bs,currentPeriod);
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
    uint32_t t = millis();
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
  BellSched *bs=CalGetDayType(Today);
  uint8_t h0 = BSGetBegHour(bs,currentPeriod);
  uint8_t m0 = BSGetBegMin(bs,currentPeriod);
  uint8_t h1 = BSGetEndHour(bs,currentPeriod);
  uint8_t m1 = BSGetEndMin(bs,currentPeriod);
  //adjust end time for countdown clock
  if (m1<countdownM)
  {
    m1+=60;
    m1-=countdownM;
    h1--;
  }
  else
  {
    m1-=countdownM;
  }
  DateTime startTime(now.year(), now.month(), now.day(), h0, m0, 0);
  DateTime endTime(now.year(), now.month(), now.day(), h1, m1, 0);
  #ifdef DEBUG3
  {
    static uint8_t LastPct;
    uint8_t CurrPct=map(now.unixtime(), startTime.unixtime(), endTime.unixtime(), 0, 100);
    if (CurrPct!=LastPct)
    {
      Serial.print("Gradient Clock: ");
      Serial.print(CurrPct);
      Serial.println("% through period");
      LastPct = CurrPct;
    }
  }
  #endif
  return map(now.unixtime(), startTime.unixtime(), endTime.unixtime(), 80, 0);
}

void displayHour(uint8_t h, uint32_t col)
{
  /* uncomment for 12 hour clock
  if (h == 0) h = 12;
  else if (h > 12) h -= 12; */
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
  BellSched *bs = CalGetDayType(Today);
  if ( Today == TODAY_IS_A_HOLIDAY || Today == TODAY_IS_A_WEEKEND ) return 0;
  else
  {
    uint8_t dl=Today%4;
    switch(dl)
    {
      case 0:
      return BSGetACol(bs,currentPeriod);
      case 1:
      return BSGetBCol(bs,currentPeriod);
      case 2:
      return BSGetCCol(bs,currentPeriod);
      case 3:
      return BSGetDCol(bs,currentPeriod);
    }
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
  // I have removed the conditional clock setting code from here.
  // I think it is better to have a dedicated sketch for setting the clock
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
  if ( dayType == dtWeekend || dayType == dtHoliday || clockType == ctBeforeSchool || clockType == ctAfterSchool ) return 65+9;
  else return CalGetDayLetter(Today);
}

void printClock()
{
  #ifdef DEBUG
  Serial.print(now.year());
  Serial.print(now.month());
  Serial.print(now.day());
  Serial.print(" ");
  Serial.print(now.hour());
  Serial.print(":");
  Serial.print(now.minute());
  Serial.print(":");
  Serial.println(now.second());
  #endif
}

/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
// CALENDAR and BELLSCHED GET DATA FUNCTIONS
/////////////////////////////////////////////////////////

uint16_t CalGetYear(uint8_t i)
{
  uint16_t offset=i*sizeof(SingleDay);
  offset += offsetof(SingleDay,Y);
  return pgm_read_word((char*)TheCalendar + offset);
}

uint8_t CalGetMonth(uint8_t i)
{
  uint16_t offset=i*sizeof(SingleDay);
  offset += offsetof(SingleDay,M);
  return pgm_read_byte((char*)TheCalendar + offset);
}

uint8_t CalGetDay(uint8_t i)
{
  uint16_t offset=i*sizeof(SingleDay);
  offset += offsetof(SingleDay,D);
  return pgm_read_byte((char*)TheCalendar + offset);
}

BellSched *CalGetDayType(uint8_t i)
{
  uint16_t offset=i*sizeof(SingleDay);
  offset += offsetof(SingleDay,dayType);
  return pgm_read_ptr((char*)TheCalendar + offset);
}

uint8_t CalGetDayLetter(uint8_t i)
{
  return i%4+65;
}

uint8_t BSGetBegHour( BellSched *BS, uint8_t P)
{
  uint16_t offset=P*sizeof(Period);
  offset += offsetof(Period,begH);
  offset += sizeof(uint8_t); //NumPeriods
  return pgm_read_byte((char*)BS+offset);
}

uint8_t BSGetBegMin( BellSched *BS, uint8_t P)
{
  uint16_t offset=P*sizeof(Period);
  offset += offsetof(Period,begM);
  offset += sizeof(uint8_t); //NumPeriods
  return pgm_read_byte((char*)BS+offset);
}

uint8_t BSGetEndHour( BellSched *BS, uint8_t P)
{
  uint16_t offset=P*sizeof(Period);
  offset += offsetof(Period,endH);
  offset += sizeof(uint8_t); //NumPeriods
  return pgm_read_byte((char*)BS+offset);
}

uint8_t BSGetEndMin( BellSched *BS, uint8_t P)
{
  uint16_t offset=P*sizeof(Period);
  offset += offsetof(Period,endM);
  offset += sizeof(uint8_t); //NumPeriods
  return pgm_read_byte((char*)BS+offset);
}

uint32_t BSGetACol( BellSched *BS, uint8_t P)
{
  uint16_t offset=P*sizeof(Period);
  offset += offsetof(Period,aCol);
  offset += sizeof(uint8_t); //NumPeriods
  return pgm_read_dword((char*)BS+offset);
}

uint32_t BSGetBCol( BellSched *BS, uint8_t P)
{
  uint16_t offset=P*sizeof(Period);
  offset += offsetof(Period,bCol);
  offset += sizeof(uint8_t); //NumPeriods
  return pgm_read_dword((char*)BS+offset);
}

uint32_t BSGetCCol( BellSched *BS, uint8_t P)
{
  uint16_t offset=P*sizeof(Period);
  offset += offsetof(Period,cCol);
  offset += sizeof(uint8_t); //NumPeriods
  return pgm_read_dword((char*)BS+offset);
}

uint32_t BSGetDCol( BellSched *BS, uint8_t P)
{
  uint16_t offset=P*sizeof(Period);
  offset += offsetof(Period,dCol);
  offset += sizeof(uint8_t); //NumPeriods
  return pgm_read_dword((char*)BS+offset);
}

uint8_t BSGetNumPeriods( BellSched *BS )
{
  uint16_t offset = offsetof(BellSched,NumPeriods);
  return pgm_read_byte((char*)BS+offset);
}
