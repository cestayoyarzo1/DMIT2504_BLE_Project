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
 
 uint8_t robot_state;
 
//tHciDataPacket robotPacket;

uint8_t command[4];
uint16_t speed;

void Robot_Init(TIM_TypeDef* _rightTimer, TIM_TypeDef* _leftTimer)
{
  rTimer = _rightTimer;
  lTimer = _leftTimer;
  robot_state = Idle;
  speed = 5;
}
//------------------------------------------------------------------------------
void Robot_SetDutyCycle(uint16_t _duty)
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
void Robot_ForwardSpeed(uint16_t _duty)
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
void Robot_ReverseSpeed(uint16_t _duty)
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
void Robot_TurnRightSpeed(uint16_t _duty)
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
void Robot_TurnLeftSpeed(uint16_t _duty)
{
  Robot_Stop();
  Robot_SetDutyCycle(_duty);  
  rTimer->CCR3 = robot_duty_cycle;
  lTimer->CCR4 = robot_duty_cycle;
}
//------------------------------------------------------------------------------
void Robot_Run()
{
  switch(robot_state)
  {
    case Idle:
      
      break;

    case MovingForward:

      break;
      
    case MovingBackwards:
      break;
      
    case TurningRight:
      
      break;

    case TurningLeft:
      
      break;    
  
  }
}
//------------------------------------------------------------------------------
 void Robot_ParseCommand(tHciDataPacket* packet)
 {

    if(packet->data_len == 16)
    {
      memcpy(command, packet->dataBuff+12, 4);
      
      switch(command[1])
      {
        case 'M': //command is for moving
          switch(command[2])
          {
            case 'F': //formward
              Robot_Forward();
              printf("\n\rMoving Forward ");  
              break;
              
            case 'B': //backwards, reverse
              Robot_Reverse();
              printf("\n\rMoving Reverse ");  

              break;  
              
            case 'R': //right
              Robot_TurnRight();
              printf("\n\rTurning Right ");

              break; 
              
            case 'L': //left
              Robot_TurnLeft();
              printf("\n\rTurning Left ");
              break; 
              
            case '0': //idle, stop
              Robot_Stop();
              printf("\n\rStopping ");
              break;               
          }
        break;
        
        case 'S': //command is for speed 
            speed = (command[2] - 48)*100;
            speed += (command[3] - 48)*10;
            Robot_SetDutyCycle(speed);
          break;
            
        default:
          break;        
      }     
    }
 }




