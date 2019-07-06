#ifndef MOTIONCONTROL_H
#define MOTIONCONTROL_H

#define SWITCH_TIMER 2000
//#define TRUE 1
//#define FALSE 0

#include <stm32l476xx.h>
#include <bluenrg_types.h>
#include <hci_tl.h>
#include <stdio.h>

  void Robot_Init(TIM_TypeDef* _rightTimer, TIM_TypeDef* _leftTimer);
  void Robot_SetDutyCycle(int  _duty);
  void Robot_Stop();
  void Robot_Forward();
  void Robot_ForwardSpeed(int _duty);
  void Robot_Reverse();
  void Robot_ReverseSpeed(int _duty);
  void Robot_TurnLeft();
  void Robot_TurnLeftSpeed(int _duty);
  void Robot_TurnRight();
  void Robot_TurnRightSpeed(int _duty);
  void Robot_Run();
  void Robot_ParseCommand(tHciDataPacket* packet);
    

              
             
#endif /* MOTIONCONTROL_H */