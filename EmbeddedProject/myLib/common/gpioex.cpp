/*
* gpioex.cpp
*
*  Created on: Sep 14, 2016
*      Author: Tomas Diaz
*/
#include <gpioex.h>

//#define DEBOUNCE 30

#define FIRST_HOLDING_LENGTH 5000
#define SECOND_HOLDING_LENGTH 15000
#define THIRD_HOLDING_LENGTH 30000
#define FOURTH_HOLDING_LENGTH 60000

void GPIOEx::Init(x_GPIO* l, bool inverted_)
{
  gpio = l;
  progress = 0;
  toggling_t = 100;
  inverted = inverted_;
  heldFlag = 0;
  pressed = false;
  debounce = 30;
  st.Start();
}

ButtonState GPIOEx::FSM()
{
  if(gpio->isOutput)
  {
    switch(pattern)
    {
    case LedPattern::Idle:
      break;
    case LedPattern::Off:
      gpio->Clear();
      pattern = LedPattern::Idle;
      break;
    case LedPattern::On:
      gpio->Set();
      pattern = LedPattern::Idle;
      break;
    case LedPattern::Beacon:
      heartbeat();
      break;
    default:
      togglingPattern();
      break;
    }
    return ButtonState::Idle;
  }
  else
  {
    ButtonState answer = ButtonState::Idle;
    bool _state = gpio->State();
    
    
    if(!pressed)
    {
      if(_state)
      {
        st.Start();
        if(st.Read()>=debounce)
        {
          pressed = true;
          pastEdge = ButtonState::Pressed;
          answer=ButtonState::Pressed;
          st.Reset();
        }
      }
      else
        st.StopAndReset();
    }
    else
    {
      if(!_state)
      {
        st.Start();
        if(st.Read()>=debounce)
        {
          pressed=false;
          pastEdge = ButtonState::Idle;
          answer=ButtonState::Released;
          heldFlag = 0;
          st.StopAndReset();
        }
      }
      else
      {
        if((st.Read()>=FOURTH_HOLDING_LENGTH) && !(heldFlag & 0x08))
        {
          answer = ButtonState::Held_Fourth_Holding_Time;
          pastEdge = ButtonState::Held_Fourth_Holding_Time;
          heldFlag |= 0x08;
        }
        else if((st.Read()>=THIRD_HOLDING_LENGTH) && !(heldFlag & 0x04))
        {
          answer = ButtonState::Held_Third_Holding_Time;
          pastEdge = ButtonState::Held_Third_Holding_Time;
          heldFlag |= 0x04;
        }
        else if((st.Read()>=SECOND_HOLDING_LENGTH) && !(heldFlag & 0x02))
        {
          answer = ButtonState::Held_Second_Holding_Time;
          pastEdge = ButtonState::Held_Second_Holding_Time;
          heldFlag |= 0x02;
        }
        else if((st.Read()>=FIRST_HOLDING_LENGTH) && !(heldFlag & 0x01))
        {
          answer = ButtonState::Held_First_Holding_Time;
          pastEdge = ButtonState::Held_First_Holding_Time;
          heldFlag |= 0x01;
        }
        else
          answer = ButtonState::Held;
      }
    }
    return answer;
  }
}

// Only valid once per debounce, false once read or button released.
ButtonState GPIOEx::PreviousEdge()
{
  return pastEdge;
}

void GPIOEx::togglingPattern()
{
  if(st.Read()>=toggling_t)
  {
    gpio->Toggle();
    st.Reset();
  }
}

void GPIOEx::SetPattern(LedPattern::Value p)
{
  if(p!=pattern)
  {
    pattern = p;
    
    progress = 0;
    gpio->Clear();
    if(p>1000)
      toggling_t = p - 1000;
    st.Reset();
    FSM();
  }
}

void GPIOEx::heartbeat()
{
  switch(progress)
  {
  case 0:
    if(st.Read()>=1500)
    {
      st.Reset();
      progress = 1;
      gpio->Set();
    }
    break;
  case 1:
    if(st.Read()>=5)
    {
      st.Reset();
      progress = 0;
      gpio->Clear();
    }
    break;
  }
}

x_GPIO* GPIOEx::GPIO()
{
  return gpio;
}
