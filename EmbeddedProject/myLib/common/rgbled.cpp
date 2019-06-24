/*
  FILE: rgbled.cpp
  Created on: 10/9/2017, by Tom Diaz
*/

#include <rgbled.h>

void RGBLed::Init(x_GPIO* red_, x_GPIO* green_, x_GPIO* blue_)
{
  led[0] = red_;
  led[1] = green_;
  led[2] = blue_;
  isOutput = true;
}
//------------------------------------------------------------------------------
void RGBLed::SetColor(RGBColor c)
{
  color = c;
}
//------------------------------------------------------------------------------
RGBColor RGBLed::GetColor()
{
  return color;
}
//------------------------------------------------------------------------------
void RGBLed::Turn(bool st)
{
  outputState = st;
  led[0]->Turn((uint8_t)color & (st ? 0x01 : 0x00));
  led[1]->Turn((uint8_t)color & (st ? 0x02 : 0x00));
  led[2]->Turn((uint8_t)color & (st ? 0x04 : 0x00));
}
//------------------------------------------------------------------------------
bool RGBLed::State()
{
  return false;
}
//------------------------------------------------------------------------------
void RGBLed::ShiftColor()
{
  switch(color)
  {
  case RGBColor::Red:
    color = RGBColor::Green;
    break;
  case RGBColor::Green:
    color = RGBColor::Blue;
    break;
  case RGBColor::Blue:
    color = RGBColor::Red;
    break;
  default:
    color = RGBColor::Red;
    break;
  }
}