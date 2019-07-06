/*
  FILE: motionControl.cpp
  Created on: 6/24/2019, by Carlos Estay
*/


#include <robot.h>

 TIM_TypeDef* rTimer;
 TIM_TypeDef* lTimer;
 int robot_duty_cycle;
 int toggleState;
 
 enum RobotState
 {
  Idle,
  MovingForward,
  MovingBackwards,
  TurningRight,
  TurningLeft,
 } ;   
 
 int state;

void Robot_Init(TIM_TypeDef* _rightTimer, TIM_TypeDef* _leftTimer)
{
  rTimer = _rightTimer;
  lTimer = _leftTimer;
  state = Idle;
}
//------------------------------------------------------------------------------
void Robot_SetDutyCycle(int _duty)
{
   robot_duty_cycle = _duty;
}
//------------------------------------------------------------------------------
void Robot_Stop()
{
  robot_duty_cycle = 0;
  rTimer->CCR3 = robot_duty_cycle;
  rTimer->CCR4 = robot_duty_cycle;
  lTimer->CCR3 = robot_duty_cycle;
  lTimer->CCR4 = robot_duty_cycle;
}
//------------------------------------------------------------------------------
void Robot_Forward()
{
  Robot_Stop();
  rTimer->CCR3 = robot_duty_cycle;
  lTimer->CCR3 = robot_duty_cycle;
}
//------------------------------------------------------------------------------
void Robot_ForwardSpeed(int _duty)
{
  Robot_Stop();
  Robot_SetDutyCycle(_duty);
  rTimer->CCR3 = robot_duty_cycle;
  lTimer->CCR3 = robot_duty_cycle;
}
//------------------------------------------------------------------------------
void Robot_Reverse()
{
  Robot_Stop();
  rTimer->CCR4 = robot_duty_cycle;
  lTimer->CCR4 = robot_duty_cycle;
}
//------------------------------------------------------------------------------
void Robot_ReverseSpeed(int _duty)
{
  Robot_Stop();
  Robot_SetDutyCycle(_duty);
  rTimer->CCR4 = robot_duty_cycle;
  lTimer->CCR4 = robot_duty_cycle;
}
//------------------------------------------------------------------------------
void Robot_TurnRight()
{
  Robot_Stop();
  rTimer->CCR4 = robot_duty_cycle;
  lTimer->CCR3 = robot_duty_cycle;  
}
//------------------------------------------------------------------------------
void Robot_TurnRightSpeed(int _duty)
{
  Robot_Stop();
  Robot_SetDutyCycle(_duty);  
  rTimer->CCR4 = robot_duty_cycle;
  lTimer->CCR3 = robot_duty_cycle;  
}
//------------------------------------------------------------------------------
void Robot_TurnLeft()
{
  Robot_Stop();
  rTimer->CCR3 = robot_duty_cycle;
  lTimer->CCR4 = robot_duty_cycle; 
}
//------------------------------------------------------------------------------
void Robot_TurnLeftSpeed(int _duty)
{
  Robot_Stop();
  Robot_SetDutyCycle(_duty);  
  rTimer->CCR3 = robot_duty_cycle;
  lTimer->CCR4 = robot_duty_cycle;
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
 void Robot_ParseCommand(tHciDataPacket* packet)
 {
    printf("\n\rReceiving Instruction");
 }




