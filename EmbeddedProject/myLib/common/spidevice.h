/*
  FILE: spidevice.h
  Created on: 9/17/2016, by Tom Diaz
*/

#ifndef SPIDEVICE_H
#define SPIDEVICE_H

#include <x_spiport.h>
#include <x_gpio.h>
#include <global_constants.h>

//---------------------------------------------------------------------SPIDevice
class SPIDevice
{
public:
  SPIDevice() { id=0; }
  virtual void Init(x_SPIPort* spi_, x_GPIO* cs_, SpiClk::Value clock_);
  void SetClock();
  uint32_t ID();
protected:
  virtual void getID();
  virtual void extraInit();
  x_SPIPort* spi;
  x_GPIO* cs;
  SpiClk::Value clock;
  uint32_t id;
};

#endif /* SPIDEVICE_H */
