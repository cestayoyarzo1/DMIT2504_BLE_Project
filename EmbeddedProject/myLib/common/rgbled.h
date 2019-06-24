/*
  FILE: $FILE_FNAME$
  Created on: 10/9/2017, by Tom Diaz
*/
#ifndef RGBLED_H
#define RGBLED_H

#include <x_gpio.h>

enum class RGBColor
{
  Red = 0x01,
  Green = 0x02,
  Blue = 0x04,
  Yellow = 0x03,
  White = 0x07,
};

class RGBLed  : public x_GPIO
{
public:
  RGBLed(){};
  ~RGBLed(){};
  void Init(x_GPIO* red_, x_GPIO* green_, x_GPIO* blue_);
  void Turn(bool st);
  void ShiftColor();
  void SetColor(RGBColor c);
  bool State();
  RGBColor GetColor();
protected:
  x_GPIO* led[3];
  //bool state[3];
  RGBColor color;
private:
};

#endif /* RGBLED_H */
