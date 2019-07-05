/*
  FILE: stm32rtc.cpp
  Created on: 9/9/2016, by Tom Diaz
*/

#include "hal.h"
#include "stm32rtc.h"

void STM32Rtc::Init(RTC_TypeDef* rtc_, PWR_TypeDef* pwr_)
{
  rtc = rtc_;
  pwr = pwr_;
}
//------------------------------------------------------------------------------
uint32_t STM32Rtc::Read()
{
  timeStruct.Year = fromBCD((rtc->DR>>16)&0xFF) + 2000;  //Year
  timeStruct.Month = fromBCD((rtc->DR>>8)&0x1F);      //Month
  timeStruct.Day = fromBCD(rtc->DR&0x3F);             //Day of the month
  timeStruct.Hour = fromBCD((rtc->TR>>16)&0x3F);  //Hours
  timeStruct.Minute = fromBCD((rtc->TR>>8)&0x7F); //Minutes
  timeStruct.Second = fromBCD(rtc->TR&0x7F);      //Seconds
  
  time = UnixTime::ConvertTo(&timeStruct); //ToSeconds(&timeStruct);
  
  return time;
}
//------------------------------------------------------------------------------
void STM32Rtc::Sync()
{
  pwr->CR1 |= PWR_CR1_DBP;              //Enable backup domain write
  rtc->WPR = 0xCA; rtc->WPR = 0x53;     //Disable rtc write protection

  rtc->ISR |= RTC_ISR_INIT;
  while(!(rtc->ISR & RTC_ISR_INITF));
  
  rtc->CR |= RTC_CR_BYPSHAD;
  
  rtc->TR = 0;
  rtc->TR |= toBCD(timeStruct.Second)<<0;     //Seconds
  rtc->TR |= toBCD(timeStruct.Minute)<<8;     //Minutes
  rtc->TR |= toBCD(timeStruct.Hour)<<16;      //Hours
  
  rtc->DR=0;
  rtc->DR |= toBCD(timeStruct.Day)<<0;        //Day of month
  rtc->DR |= toBCD(timeStruct.Month)<<8;      //Month
  rtc->DR |= toBCD(timeStruct.Year-2000)<<16;      //Year
  
  //rtc->DR |= 1;/*toBCD(src[6])<<13;*/    //Day of week
  
  rtc->CR &= ~RTC_CR_FMT;
  
  rtc->ISR &= ~RTC_ISR_INIT;

  rtc->WPR =0xFF;               //Enable rtc write protection
  pwr->CR1 &= ~PWR_CR1_DBP;     //Disable backup domain write
}

