/*
  FILE: timehandler.cpp
  Created on: 9/9/2016, by Tom Diaz
*/

#include "timehandler.h"

/*void TimeHandler::Init(x_RTC* rtc_, uint32_t* time_)
{
  rtc = rtc_;
  timePtr = time_;
}*/

void TimeHandler::SetSource(x_RTC* rtc_)
{
  rtc = rtc_;
}

void TimeHandler::SetDestination(uint32_t* time_)
{
  timePtr = time_;
}
  
void TimeHandler::SyncRTC(uint32_t time_)
{
  rtc->Sync(time_);
}

void TimeHandler::fsm()
{
  *timePtr = rtc->Read();
}

x_RTC* TimeHandler::Rtc()
{
  return rtc;
}
