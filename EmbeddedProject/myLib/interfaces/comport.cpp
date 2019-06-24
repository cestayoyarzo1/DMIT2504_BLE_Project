/*
 * comport.cpp
 *
 *  Created on: Sep 15, 2016
 *      Author: Tomas Diaz
 */

#include <hal.h>	//Platform dependent
#include <comport.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>


//------------------------------------------------------------------------------
bool COMPort::IsCmdComplete()
{
  if(mode==CommandMode::NewLine && tOut.Read()>=2000)
  {
    tOut.StopAndReset();
    RxBuffer->Clear();
  }
  else if(mode==CommandMode::Timeout && tOut.Read()>=5)
  {
    tOut.StopAndReset();
    cmdComplete=true;
  }

  if(cmdComplete)
  {
    RxBuffer->Close();
    cmdComplete=false;
    return true;
  }
  return false;
}
//------------------------------------------------------------------------------
void COMPort::Printf(const char* format, ...)
{   
  va_list arg; 
  va_start(arg, format); 
  TxBuffer->Clear();
  TxBuffer->SPrintf(format,arg);
  va_end(arg);
  FlushTx();
}
//------------------------------------------------------------------------------
void COMPort::Printf(const char* format, __Va_list l)
{
  /*uint32_t len = strlen(format);
  if(len <  TxBuffer->Size())
  {
    vsprintf((char*)TxBuffer->GetPointer(0),format,l);
    TxBuffer->Adjust();
  }
  else
  {
    TxBuffer->Clear();
    TxBuffer->AddArray((void*)"Printf buffer too big");
  }*/
  TxBuffer->Clear();
  TxBuffer->SPrintf(format,l);
  FlushTx();
}
//------------------------------------------------------------------------------
COMId::Value COMPort::Id()
{
  return id;
}
//------------------------------------------------------------------------------
void COMPort::WriteString(uint8_t* str)
{
  uint32_t length = strlen((const char*)str);
  if(length)
    writeString(str,length);
}
//------------------------------------------------------------------------------
void COMPort::WriteString(const char* str)
{
  uint32_t length = strlen(str);
  writeString((uint8_t*)str,length);
}
//------------------------------------------------------------------------------
void COMPort::WriteString(uint8_t* str, uint16_t length)
{
  writeString(str,length);
}
//------------------------------------------------------------------------------
void COMPort::Echo()
{
  RxBuffer->AddCr();
  WriteString(RxBuffer->GetPointer(0),RxBuffer->Length());
}
//------------------------------------------------------------------------------
void COMPort::SetCommandMode(CommandMode::Value m)
{
  mode = m;
}
//------------------------------------------------------------------------------
void COMPort::FlushTx()
{
  writeString(TxBuffer->GetPointer(0),TxBuffer->Length());
}
//------------------------------------------------------------------------------
BufferHandler* COMPort::TxB()
{
  return TxBuffer;
}
//------------------------------------------------------------------------------
BufferHandler* COMPort::RxB()
{
  return RxBuffer;
}
