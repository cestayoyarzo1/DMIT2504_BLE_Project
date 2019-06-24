/*
 * comhandler.cpp
 *
 *  Created on: Sep 16, 2016
 *      Author: Tom Diaz
 */

#include <hal.h>
#include <comhandler.h>

//-------------------------------------------------------------------------fsm()
void COMHandler::fsm()
{
  for(uint_fast8_t i = 0; i<activePorts; i++)
  {
    if(port[i]->IsCmdComplete())
    {
      callback->Args[0] = (intptr_t)port[i];
      callback->Args[1] = i;
      callback->Process(Process::Parse_COMPort_Message);
      port[i]->RxB()->Clear();
    }
  }
}
//-----------------------------------------------------------------------AddPort
void COMHandler::Add(COMPort* p)
{
  port[activePorts++] = p;
}
//-----------------------------------------------------------------------GetPort
COMPort* COMHandler::Get(uint32_t p)
{
  p = p > activePorts ? 0 : p;
  return port[p];
}
