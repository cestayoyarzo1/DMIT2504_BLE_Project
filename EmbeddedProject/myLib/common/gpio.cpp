/*
 * gpio.cpp
 *
 *  Created on: Sep 14, 2016
 *      Author: Tomas Diaz
 */

#include <hal.h>	// Platform dependent
#include <gpio.h>

void GPIO::InitAsOutput(GPIO_TypeDef* p, uint16_t b, bool iState)
{
  isOutput = true;
  port = p;
  bit = b;
  port->MODER &= ~(3<<(bit*2)); //Clear pin current configuration
  port->MODER |= 1<<(bit*2);    //Set as output
  port->BSRR |= 1<<(bit+16);    //Set output low
  port->PUPDR &= ~(3<<(bit*2)); //No pull-up or pull-low
  port->PUPDR |= 2<<(bit*2);
  Turn(iState);
  
 
  //HumPRO temporary patch to check if an open collector is needed
  if (p == GPIOA && b == 4)
  {
  port->MODER &= ~(3<<(bit*2)); //Clear pin current configuration
  port->MODER |= 1<<(bit*2);    //Set as output
  
  port->OTYPER |= 0x10;         //open collector
  
  port->BSRR |= 1<<(bit+16);    //Set output low
  
  port->PUPDR &= ~(3<<(bit*2)); //No pull-up or pull-low
  //port->PUPDR |= 2<<(bit*2);
  port->PUPDR |= 1<<(bit*2);    //pull-up  
  Turn(iState);
    
  }

}

void GPIO::InitAsInput(GPIO_TypeDef* p, uint16_t b, Resistor::Value r)
{
  isOutput = false;
  port = p;
  bit = b;
  resistor = r;
  port->MODER &= ~(3<<(bit*2)); //Clear current pin configuration
  port->PUPDR &= ~(3<<(bit*2)); //No pull-up or pull-low

  switch(resistor)              //Set pull-up or pull-low
  {
  case Resistor::None: break;
  case Resistor::ExtPullUp: resistor = Resistor::Pullup; break;
  case Resistor::Pullup: port->PUPDR |= 1<<(bit*2); break;
  case Resistor::Pulldown: port->PUPDR |= 2<<(bit*2); break;
  }
}

void GPIO::Turn(bool state)
{
  if(isOutput)
  {
    outputState = state;
    if(state)
    {
      if(inverted)
        port->BSRR |= 1<<(bit+16);
      else
        port->BSRR |= 1<<bit;
    }
    else
    {
      if(inverted)
        port->BSRR |= 1<<bit;
      else
        port->BSRR |= 1<<(bit+16);
    }
  }
}

bool GPIO::State()
{
  bool s = port->IDR&(1<<bit);
  return (resistor==Resistor::Pullup) ? !s : s;
}

