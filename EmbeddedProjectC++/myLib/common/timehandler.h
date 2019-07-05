/*
  FILE: timehandler.h
  Created on: 9/9/2016, by Tom Diaz
*/

#ifndef TIMEHANDLER_H
#define TIMEHANDLER_H

#include "x_rtc.h"
#include "x_handler.h"
//#include "support.h"

class TimeHandler : public x_Handler
{
public:
  TimeHandler(){};
  ~TimeHandler(){};
  //void Init(x_RTC* rtc_, uint32_t* time_);
  void SetSource(x_RTC* rtc_);
  void SetDestination(uint32_t* time_);
  void SyncRTC(uint32_t time_);
  void fsm();
  x_RTC* Rtc();
protected:
private:
  x_RTC* rtc;
  uint32_t* timePtr;
};


#endif /* TIMEHANDLER_H */
