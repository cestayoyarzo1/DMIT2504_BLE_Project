/*
 * x_gpio.cpp
 *
 *  Created on: Sep 14, 2016
 *      Author: Tomas Diaz
 */

#include <x_gpio.h>

void x_GPIO::Clear()
{
  Turn(false);
}

void x_GPIO::Set()
{
  Turn(true);
}

void x_GPIO::Toggle()
{
  if(!outputState)
    Turn(true);
  else
    Turn(false);
}

void x_GPIO::SetInverted(bool val)
{
  inverted = val;
}