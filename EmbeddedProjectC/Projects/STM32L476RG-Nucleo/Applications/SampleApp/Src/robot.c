/*
  FILE: motionControl.cpp
  Created on: 6/24/2019, by Carlos Estay
*/


#include <robot.h>


void Robot_Init(TIM_TypeDef* _rightTimer, TIM_TypeDef* _leftTimer)
{
  rightTimer = _rightTimer;
  leftTimer = _leftTimer;
  state = Idle;
}
//------------------------------------------------------------------------------
void Robot_SetDutyCycle(int _duty)
{
   duty = _duty;
}
//------------------------------------------------------------------------------
void Robot_Stop()
{
  duty = 0;
  rightTimer->CCR3 = duty;
  rightTimer->CCR4 = duty;
  leftTimer->CCR3 = duty;
  leftTimer->CCR4 = duty;
}
//------------------------------------------------------------------------------
void Robot_Forward()
{
  Robot_Stop();
  rightTimer->CCR3 = duty;
  leftTimer->CCR3 = duty;
}
//------------------------------------------------------------------------------
void Robot_ForwardSpeed(int _duty)
{
  Robot_Stop();
  Robot_SetDutyCycle(_duty);
  rightTimer->CCR3 = duty;
  leftTimer->CCR3 = duty;
}
//------------------------------------------------------------------------------
void Robot_Reverse()
{
  Robot_Stop();
  rightTimer->CCR4 = duty;
  leftTimer->CCR4 = duty;
}
//------------------------------------------------------------------------------
void Robot_ReverseSpeed(int _duty)
{
  Robot_Stop();
  Robot_SetDutyCycle(_duty);
  rightTimer->CCR4 = duty;
  leftTimer->CCR4 = duty;
}
//------------------------------------------------------------------------------
void Robot_TurnRight()
{
  Robot_Stop();
  rightTimer->CCR4 = duty;
  leftTimer->CCR3 = duty;  
}
//------------------------------------------------------------------------------
void Robot_TurnRightSpeed(int _duty)
{
  Robot_Stop();
  Robot_SetDutyCycle(_duty);  
  rightTimer->CCR4 = duty;
  leftTimer->CCR3 = duty;  
}
//------------------------------------------------------------------------------
void Robot_TurnLeft()
{
  Robot_Stop();
  rightTimer->CCR3 = duty;
  leftTimer->CCR4 = duty; 
}
//------------------------------------------------------------------------------
void Robot_TurnLeftSpeed(int _duty)
{
  Robot_Stop();
  Robot_SetDutyCycle(_duty);  
  rightTimer->CCR3 = duty;
  leftTimer->CCR4 = duty; 
}
//------------------------------------------------------------------------------
void Robot_Run()
{
  switch(state)
  {
    case Idle:
      if(toggleState)
      {
        Robot_ForwardSpeed(500);
        toggleState = FALSE;
        state = MovingForward;
      }
      break;

    case MovingForward:
      if(toggleState)
      {
        Robot_ReverseSpeed(500);
        toggleState = FALSE;
        state = MovingBackwards;
      }
      break;
      
    case MovingBackwards:
      if(toggleState)
      {
        Robot_TurnRightSpeed(250);
        toggleState = FALSE;
        state = TurningRight;
      }
      break;
      
    case TurningRight:
      if(toggleState)
      {
        Robot_TurnLeftSpeed(250);
        toggleState = FALSE;
        state = TurningLeft;
      }      
      break;

    case TurningLeft:
      if(toggleState)
      {
        Robot_Stop();
        toggleState = FALSE;
        state = Idle;
      }       
      break;    
  
  }
}
//------------------------------------------------------------------------------





