/*
  FILE: stm32rtc.h
  Created on: 9/9/2016, by Tom Diaz
  stm32rtc header file for Vcu5x / STM32L476
*/

#ifndef STM32RTC_H
#define STM32RTC_H

#include "x_rtc.h"

class STM32Rtc : public x_RTC
{
public:
  STM32Rtc(){};
  ~STM32Rtc(){};
  void Init(RTC_TypeDef* rtc_, PWR_TypeDef* pwr_);
  uint32_t Read();  //---Overriden
  void Sync();  //---Overriden 
private:
  RTC_TypeDef* rtc;
  PWR_TypeDef* pwr;
};


#endif /* STM32RTC_H */
