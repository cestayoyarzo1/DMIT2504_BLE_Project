#ifndef MOTIONCONTROL_H
#define MOTIONCONTROL_H



class MotionControl
{
  

public:
  void Init(TIM_TypeDef* _rightTimer, TIM_TypeDef* _leftTimer);
  void SetDutyCycle(uint16_t  _duty);
  void Stop();
  void Forward();
  void Forward(uint16_t _duty);
  void Reverse();
  void Reverse(uint16_t _duty);
  void TurnLeft();
  void TurnLeft(uint16_t _duty);
  void TurnRight();
  void TurnRight(uint16_t _duty);

    
private:
   TIM_TypeDef* rightTimer;
   TIM_TypeDef* leftTimer;
   uint16_t duty;
    
};
              
             
#endif /* MOTIONCONTROL_H */