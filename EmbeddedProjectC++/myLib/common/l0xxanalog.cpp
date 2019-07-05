/*
  FILE: l051analog..cpp
  Created on: 11/22/2017, by Tom Diaz
*/

#include <hal.h>
#include <l0xxanalog.h>

L0xxAnalogHandler::L0xxAnalogHandler()
{
}
//------------------------------------------------------------------------------
void L0xxAnalogHandler::Init(uint16_t cal_)
{
  cal = cal_;
}
//------------------------------------------------------------------------------
void L0xxAnalogHandler::fsm()
{
  uint16_t vddaR = channel[ADCChannel::VrefInt];
  uint16_t tsR = channel[ADCChannel::Ts];
  int16_t ts1 = 0;
  int16_t ts2 = 0;
  
  
  // Multiply everything by 100 to scale it and avoid floating point math.
  vdda = 300 * (VREFINT_CAL * 100) / ((uint16_t)vddaR * 100);
  
  ts1 = 300 * (TS_CAL1 * 100) / (vdda * 100);
  ts2 = 300 * (TS_CAL2 * 100) / (vdda * 100);
  
  temperature =  ((10000 / (ts2 - ts1)) * (tsR - ts1)) + 3000;
  
  callback->Args[0] = (intptr_t)temperature / 100;
  callback->Set(Set::Temperature);
}
//------------------------------------------------------------------------------
void L0xxAnalogHandler::SetData(uint16_t* data)
{
  for(uint32_t i = 0; i < ADCChannel::Max; i++)
  {
    channel[i] = data[i];
  }
}