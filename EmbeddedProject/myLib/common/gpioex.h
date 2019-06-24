/*
* gpioex.h
*
*  Created on: Sep 14, 2016
*      Author: Tom Diaz
*/

#ifndef GPIOEX_H_
#define GPIOEX_H_

#include <stdint.h>
#include <x_gpio.h>
#include <stopwatch.h>

// Emulates C++11's enum class
class LedPattern
{
public:
  enum Value
  {
    Off = 0,
    On = 1,
    Idle = 2,
    Beacon = 3,
    Toggle_1ms = 1001,
    Toggle_5ms = 1005,
    Toggle_10ms = 1010,
    Toggle_25ms = 1025,
    Toggle_100ms = 1100,
    Toggle_500ms = 1500,
    Toggle_1s = 2000,
  };
};

enum class ButtonState
{
  Idle,
  Pressed,
  Released,
  Held,
  Held_Fourth_Holding_Time,
  Held_Third_Holding_Time,
  Held_Second_Holding_Time,
  Held_First_Holding_Time,
};

class GPIOEx
{
public:
  ButtonState FSM();
  void Init(x_GPIO* l, bool inverted_);
  void SetPattern(LedPattern::Value p);
  x_GPIO* GPIO();
  void SetDebounce(uint32_t d) { debounce = d; }
  ButtonState PreviousEdge(); // Pressed and debounced
private:
protected:
  x_GPIO* gpio;
  uint8_t heldFlag;
  LedPattern::Value pattern;
  uint32_t progress;
  Stopwatch st;
  uint32_t toggling_t;
  void togglingPattern();
  void heartbeat();
  //bool debounced;
  ButtonState pastEdge;
  bool pressed;
  bool inverted;
  uint32_t debounce;
};


#endif /* GPIOEX_H_ */
