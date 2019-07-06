#ifndef MOTIONCONTROL_H
#define MOTIONCONTROL_H

#define SWITCH_TIMER 2000
#define TRUE 1
#define FALSE 0

#include <stm32l476xx.h>

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
    
   TIM_TypeDef* rightTimer;
   TIM_TypeDef* leftTimer;
   int duty;
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
              
             
#endif /* MOTIONCONTROL_H */