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
 
 uint8_t robot_state, prev_state;
 
//tHciDataPacket robotPacket;

uint8_t command[4];
uint16_t speed;

void Robot_Init(TIM_TypeDef* _rightTimer, TIM_TypeDef* _leftTimer)
{
  rTimer = _rightTimer;
  lTimer = _leftTimer;
  robot_state = prev_state = Idle;
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
  robot_state = Idle;
}
//------------------------------------------------------------------------------
void Robot_Forward()
{
  Robot_Stop();
  rTimer->CCR3 = robot_duty_cycle;
  lTimer->CCR3 = robot_duty_cycle;
  robot_state = MovingForward;
}
//------------------------------------------------------------------------------
void Robot_ForwardSpeed(uint16_t _duty)
{
  Robot_Stop();
  Robot_SetDutyCycle(_duty);
  rTimer->CCR3 = robot_duty_cycle;
  lTimer->CCR3 = robot_duty_cycle;
  robot_state = MovingForward;
}
//------------------------------------------------------------------------------
void Robot_Reverse()
{
  Robot_Stop();
  rTimer->CCR4 = robot_duty_cycle;
  lTimer->CCR4 = robot_duty_cycle;
  robot_state = MovingBackwards;
}
//------------------------------------------------------------------------------
void Robot_ReverseSpeed(uint16_t _duty)
{
  Robot_Stop();
  Robot_SetDutyCycle(_duty);
  rTimer->CCR4 = robot_duty_cycle;
  lTimer->CCR4 = robot_duty_cycle;
  robot_state = MovingBackwards;
}
//------------------------------------------------------------------------------
void Robot_TurnRight()
{
  Robot_Stop();
  rTimer->CCR4 = robot_duty_cycle;
  lTimer->CCR3 = robot_duty_cycle; 
  robot_state = TurningRight;
}
//------------------------------------------------------------------------------
void Robot_TurnRightSpeed(uint16_t _duty)
{
  Robot_Stop();
  Robot_SetDutyCycle(_duty);  
  rTimer->CCR4 = robot_duty_cycle;
  lTimer->CCR3 = robot_duty_cycle; 
  robot_state = TurningRight;
}
//------------------------------------------------------------------------------
void Robot_TurnLeft()
{
  Robot_Stop();
  rTimer->CCR3 = robot_duty_cycle;
  lTimer->CCR4 = robot_duty_cycle;
  robot_state = TurningLeft;
}
//------------------------------------------------------------------------------
void Robot_TurnLeftSpeed(uint16_t _duty)
{
  Robot_Stop();
  Robot_SetDutyCycle(_duty);  
  rTimer->CCR3 = robot_duty_cycle;
  lTimer->CCR4 = robot_duty_cycle;
  robot_state = TurningLeft;
}
//------------------------------------------------------------------------------
void Robot_Run()
{
  switch(robot_state)
  {
    case Idle:
      if(robot_state != prev_state)
      {
        prev_state = robot_state;
        printf("Idle");
      }
      break;

    case MovingForward:
      if(robot_state != prev_state)
      {
        prev_state = robot_state;
        printf("Forward");
      }
      break;
      
    case MovingBackwards:
      if(robot_state != prev_state)
      {
        prev_state = robot_state;
        printf("Reverse");
      }    
      break;
      
    case TurningRight:
      if(robot_state != prev_state)
      {
        prev_state = robot_state;
        printf("Turn Right");
      }     
      break;

    case TurningLeft:
      if(robot_state != prev_state)
      {
        prev_state = robot_state;
        printf("Turn Left");
      }        
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
              break;
              
            case 'B': //backwards, reverse
              Robot_Reverse();

              break;  
              
            case 'R': //right
              Robot_TurnRight();

              break; 
              
            case 'L': //left
              Robot_TurnLeft();
              break; 
              
            case '0': //idle, stop
              Robot_Stop();
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




