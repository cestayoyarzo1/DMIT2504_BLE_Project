#include <spbtle.h>


void SPBTLEBt::Init(x_SPIPort* spi_, x_GPIO* cs_, SpiClk::Value clock_, x_GPIO* reset_)
{
  this->SPIDevice::Init(spi_, cs_, clock_);
  reset = reset_;
}
//------------------------------------------------------------------------------
void  SPBTLEBt::Reset()
{
  
}
//------------------------------------------------------------------------------