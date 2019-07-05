/*
  FILE: x_gpio.h
  Created on: 6/29/2016, by Tom Diaz
*/

#ifndef X_GPIO_H
#define X_GPIO_H

#include <stdint.h>

// Emulates C++11's enum class
class Resistor
{
public:
  enum Value
  {
    None,
    Pullup,
    ExtPullUp,
    Pulldown,
  };
};

class x_GPIO
{
friend class GPIOEx;
public:
  void Set();
  void Clear();
  virtual void Turn(bool state) = 0;
  void Toggle();
  void SetInverted(bool val);
  virtual bool State() = 0;
protected:
  bool inverted;
  bool outputState;
  uint8_t bit;
  uint8_t isOutput;
  Resistor::Value resistor;
};

#endif /* X_GPIO_H */
