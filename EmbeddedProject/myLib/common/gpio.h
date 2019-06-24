/*
 * gpio.h
 *
 *  Created on: Sep 14, 2016
 *      Author: Tomas Diaz
*/

#ifndef GPIO_H_
#define GPIO_H_

#include <stdint.h>
#include <x_gpio.h>

class GPIO : public x_GPIO
{
public:
  void InitAsOutput(GPIO_TypeDef* p, uint16_t b, bool iState);
  void InitAsInput(GPIO_TypeDef* p, uint16_t b, Resistor::Value r);
  void Turn(bool state);
  bool State();
private:
protected:
  GPIO_TypeDef* port;
};

#endif /* GPIO_H_ */
