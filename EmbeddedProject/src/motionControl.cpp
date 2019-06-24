/*
  FILE: motionControl.cpp
  Created on: 6/24/2019, by Carlos Estay
*/


#include <hal.h>


void MotionControl::Init(TIM_TypeDef* _rightTimer, TIM_TypeDef* _leftTimer)
{
  rightTimer = _rightTimer;
  leftTimer = _leftTimer;
}
//------------------------------------------------------------------------------
void MotionControl::SetDutyCycle(uint16_t _duty)
{
   duty = _duty;
}
//------------------------------------------------------------------------------
void MotionControl::Stop()
{
  duty = 0;
  rightTimer->CCR3 = duty;
  rightTimer->CCR4 = duty;
  leftTimer->CCR3 = duty;
  leftTimer->CCR4 = duty;
}
//------------------------------------------------------------------------------
void MotionControl::Forward()
{
  Stop();
  rightTimer->CCR3 = duty;
  leftTimer->CCR3 = duty;
}
//------------------------------------------------------------------------------
void MotionControl::Forward(uint16_t _duty)
{
  Stop();
  SetDutyCycle(_duty);
  rightTimer->CCR3 = duty;
  leftTimer->CCR3 = duty;
}
//------------------------------------------------------------------------------
void MotionControl::Reverse()
{
  Stop();
  rightTimer->CCR4 = duty;
  leftTimer->CCR4 = duty;
}
//------------------------------------------------------------------------------
void MotionControl::Reverse(uint16_t _duty)
{
  Stop();
  SetDutyCycle(_duty);
  rightTimer->CCR4 = duty;
  leftTimer->CCR4 = duty;
}
//------------------------------------------------------------------------------
void MotionControl::TurnRight()
{
  Stop();
  rightTimer->CCR4 = duty;
  leftTimer->CCR3 = duty;  
}
//------------------------------------------------------------------------------
void MotionControl::TurnRight(uint16_t _duty)
{
  Stop();
  SetDutyCycle(_duty);  
  rightTimer->CCR4 = duty;
  leftTimer->CCR3 = duty;  
}
//------------------------------------------------------------------------------
void MotionControl::TurnLeft()
{
  Stop();
  rightTimer->CCR3 = duty;
  leftTimer->CCR4 = duty; 
}
//------------------------------------------------------------------------------
void MotionControl::TurnLeft(uint16_t _duty)
{
  Stop();
  SetDutyCycle(_duty);  
  rightTimer->CCR3 = duty;
  leftTimer->CCR4 = duty; 
}
//------------------------------------------------------------------------------




