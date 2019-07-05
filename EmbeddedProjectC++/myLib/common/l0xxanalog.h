/*
  FILE: l051analog.h
  Created on: 11/22/2017, by Tom Diaz
*/
#ifndef L051_ANALOG_HANDLER_H
#define L051_ANALOG_HANDLER_H

#include <x_handler.h>

class L0xxAnalogHandler : public x_Handler
{
public:
  L0xxAnalogHandler();
  ~L0xxAnalogHandler(){};
  void Init(uint16_t cal_);
  void SetData(uint16_t* data);
  void fsm();
protected:
  uint16_t vdda;
  uint16_t temperature;
  uint16_t cal;
  uint16_t channel[ADCChannel::Max];
private:
};


#endif /* L051_ANALOG_HANDLER_H */

