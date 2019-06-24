/*
  FILE: connectionhandler.cpp
  Created on: 9/19/2016, by Tom Diaz
*/

#include <hal.h>
#include <connectionhandler.h>

void ConnectionHandler::extraInit()
{
  bool tmpState = sys->Get(Get::Uart2_Valid);
  phyState = tmpState ? ConnectionSource::RS232 : ConnectionSource::STLink;
  HAL::SwitchU2Tx(phyState);
}
//------------------------------------------------------------------------------
ConnectionSource::Value ConnectionHandler::PHY()
{
  return phyState;
}
//------------------------------------------------------------------------------
ConnectionSource::Value ConnectionHandler::Connected()
{
  return connected;
}
//------------------------------------------------------------------------------
void ConnectionHandler::handleTimeout()
{
  if(count>=500)
  {
    count = 0;
    connected = ConnectionSource::None;
  }
  else
    count++;
}
//------------------------------------------------------------------------------
void ConnectionHandler::Ping(ConnectionSource::Value c)
{
  count = 0;
  connected = c;
}
//------------------------------------------------------------------------------
void ConnectionHandler::fsm() // Called by parent Root::FSM
{
  ButtonState::Value r232State = 
    (ButtonState::Value)sys->Get(Get::Uart2_Valid_Edge);
  switch(r232State)
  {
  case ButtonState::Pressed:
    phyState = ConnectionSource::RS232;
    HAL::SwitchU2Tx(phyState);
    sys->Args[0] = (intptr_t)"@NX\r";
    sys->Set(Set::Uart_Write_String);
    break;
  case ButtonState::Released:
    phyState = ConnectionSource::STLink;
    HAL::SwitchU2Tx(phyState);
    break;
  }
  
  if(phyState == ConnectionSource::RS232)
  {
    if(pingCount>=250)
    {
      sys->Args[0] = (intptr_t)"@NX\r";
      sys->Set(Set::Uart_Write_String);
      pingCount = 0;
    }
    else
      pingCount+=1;;
  }
  
  /*if(!connPast && connected) // Connecion was established
    s->DisplayHandler.SetConnection(phyState);
  else if(connPast && !connected)
    s->DisplayHandler.SetConnection(ConnectionSource::None);*/
  
  connPast = connected;
  
  handleTimeout();
}