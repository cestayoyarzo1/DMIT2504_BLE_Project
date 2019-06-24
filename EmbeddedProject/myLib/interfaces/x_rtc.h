/*
  FILE: x_rtc.h
  Created on: 9/9/2016, by Tom Diaz
*/

#ifndef X_RTC_H
#define X_RTC_H

#include "stdint.h"
#include "unixtime.h"
/*typedef struct
{
    uint8_t second; // 0-59
    uint8_t minute; // 0-59
    uint8_t hour;   // 0-23
    uint8_t day;    // 1-31
    uint8_t month;  // 1-12
    uint8_t year;   // 0-99 (representing 2000-2099)
}
DateTime_c;*/

class x_RTC
{
public:
  x_RTC();
  ~x_RTC(){};
  //virtual void Read(DateTime_c* dst_) = 0;  //---Interface
  virtual uint32_t Read() = 0;
  virtual void Sync() = 0;                    //---Interface 
  void Sync(uint32_t time_);
  DateTime* TimeStruct();
  void EnableStringDate(bool state);
protected:
  uint8_t fromBCD(uint8_t data);
  uint8_t toBCD(uint8_t data);
  DateTime timeStruct;
  uint32_t time; // Seconds after Jan 1 2001 00:00
  //bool strEnabled;
  uint8_t DateString[32];
private:
};


#endif /* X_RTC_H */
