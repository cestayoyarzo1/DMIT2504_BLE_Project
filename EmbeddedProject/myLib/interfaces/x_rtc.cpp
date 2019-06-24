/*
  FILE: x_rtc.cpp
  Created on: 9/9/2016, by Tom Diaz
*/

#include "x_rtc.h"
#include "string.h"
#include "stdio.h"

x_RTC::x_RTC()
{
  memcpy(DateString, (void*)"Disabled",9); 
}
//------------------------------------------------------------------------------
uint8_t x_RTC::fromBCD(uint8_t val)
{
  uint8_t h=0,l=0;
  h=(val>>4)*10;
  l=val&0x0F;
  return(l+h);
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
uint8_t x_RTC::toBCD(uint8_t val)
{
  uint8_t h=0,l=0,bcd=0;
 
  h=val/10;
  l=val-(h*10);
  bcd=l+(h<<4);
  return(bcd);
}
//------------------------------------------------------------------------------
void x_RTC::Sync(uint32_t time_)
{
  //ToDateTime(&timeStruct,time_);
  timeStruct = UnixTime::ConvertFrom(time_);
  Sync();
}
//------------------------------------------------------------------------------
DateTime* x_RTC::TimeStruct()
{
  return &timeStruct;
}

