/*
  FILE: spidevice.cpp
  Created on: 9/17/2016, by Tom Diaz
*/

#include <hal.h>
#include <spidevice.h>


void SPIDevice::Init(x_SPIPort* spi_, x_GPIO* cs_, SpiClk::Value clock_)
{
  cs_->Set();
  spi = spi_;
  cs = cs_;
  clock = clock_;
  SetClock();
  getID();
  extraInit();
}
//------------------------------------------------------------------------------
void SPIDevice::getID()
{
}
//------------------------------------------------------------------------------
void SPIDevice::SetClock()
{
  spi->SetClock(clock);
}
//------------------------------------------------------------------------------
uint32_t SPIDevice::ID()
{
  return id;
}
//------------------------------------------------------------------------------
void SPIDevice::extraInit()
{
  
}