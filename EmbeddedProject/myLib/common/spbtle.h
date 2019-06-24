/*
  FILE: spbtle.h
  Created on: 6/24/2019, by Carlos Estay
*/

#ifndef SPBTLEBT_H
#define SPBTLEBT_H

#include <spidevice.h>
#include <x_gpio.h>
#include <stopwatch.h>

class SPBTLEBt: SPIDevice
{
  
public:
  void  Init(x_SPIPort* spi_, x_GPIO* cs_, SpiClk::Value clock_, x_GPIO* reset_);
  void Reset();
  
private:
  x_GPIO* reset;
  Stopwatch timer;
};

#endif /* SPBTLEBT_H */
