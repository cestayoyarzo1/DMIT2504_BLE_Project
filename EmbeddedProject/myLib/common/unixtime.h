/*
  FILE: unixtime.h
  Created on: 8/16/2017, by Tom Diaz
*/
#ifndef UNIXTIME_H
#define UNIXTIME_H

#include <stdint.h>
#include <string.h>

// Data structure for DateTime
class DateTime 
{
public:
  DateTime& operator=(DateTime src)
  {
    memcpy(this,&src,sizeof(DateTime));
    return *this;
  }
  
  DateTime& operator=(DateTime* src)
  {
    memcpy(this,src,sizeof(DateTime));
    return *this;
  }
  uint16_t Year;
  uint8_t Month;
  uint8_t Day;
  uint8_t Hour;
  uint8_t Minute;
  uint8_t Second;
};

class UnixTime
{
public:
  static uint32_t ConvertTo(const DateTime *dateTime);
  static DateTime* ConvertFrom(uint32_t unixTime);
private:
  static uint16_t GetDayNumFromYearStart(uint8_t day, uint8_t month, bool isLeapYear);
  static uint8_t GetMonthByDay(uint16_t day, bool isLeapYear);
  static uint8_t GetMonthDayByYearDay(uint16_t yearDay, bool isLeapYear);
  static bool isLeapYear(uint16_t year);
  static uint8_t daysInMonth(uint8_t month, bool isLeapYear);
  static DateTime tm;
  static const uint32_t SecondsInNonLeapYear = 31536000;
  static const uint32_t SecondsInLeapYear    = 31622400;
  static const uint32_t HoursInYear          = 31436000;
  static const uint32_t SecondsInDay         = 86400;	
  static const uint16_t SecondsInHour        = 3600;
  static const uint16_t MinutesInDay         = 1440;
  static const uint8_t  SecondsInMinute      = 60;
};

#endif /* UNIXTIME_H */
