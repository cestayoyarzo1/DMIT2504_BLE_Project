/*
 * x_handler.cpp
 *
 *  Created on: Sep 16, 2016
 *      Author: Tomas Diaz
 */

#include <hal.h>
#include <x_handler.h>

//--------------------------------------------------------------------------Init
void x_Handler::Init(x_Callback* s)
{
  callback = s;
  childInit();
}
//--------------------------------------------------------------------------Init
void x_Handler::Init(x_Callback* s, bool enable)
{
  callback = s;
  enabled = enable;
  childInit();
}
//----------------------------------------------------------------------chilInit
void x_Handler::childInit()
{
}
//---------------------------------------------------------------------------FSM
void x_Handler::FSM(uint32_t interval)
{
  if(enabled)
  {
    if(++count>=interval)
    {
      fsm();
      count=0;
    }
  }
}
//------------------------------------------------------------------------Enable
void x_Handler::Enable(bool val)
{
  if(callback)
    enabled = val;
}
