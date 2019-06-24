/*
  FILE: digi3g.cpp
  Created on: 9/7/2017, by Tom Diaz
*/

#include <hal.h>
#include <digi3g.h>
#include <stdio.h>
#include <string.h>

static const char* AT = "AT";
static const char* DL = "DL";
static const char* DE = "DE";
static const char* AI = "AI";
static const char* SM = "SM";
static const char* CRLF = "\r\n";

void Digi3G::Init(UART* uart_, x_GPIO* reset_, BufferHandler* handler_, HttpParser* parser_)
{
  uart = uart_;
  handler = handler_;
  parser = parser_;
  fsmState = State::ResetBegin;
  associated = false;
  reset = reset_;
}
//------------------------------------------------------------------------------
void Digi3G::makePortStr(uint16_t port_)
{
  sprintf((char*)port,"%x",port_);
}
//------------------------------------------------------------------------------
void Digi3G::SetHost(uint8_t* host_, uint8_t* route_, uint16_t port_, uint8_t protocol_)
{
  host = host_;
  route = route_;
  makePortStr(port_);
  protocol[0] = protocol_ < 5 ? protocol_ + 0x30 : 0x31;
  fsmState = State::Init;
}
//------------------------------------------------------------------------------
void Digi3G::setATCommand(ATCommand cmd)
{
  fsmState = State::Command;
  atCommand = cmd;
}
//------------------------------------------------------------------------------
void Digi3G::FSM()
{
  switch(fsmState)
  {
  case State::ResetBegin:
    reset->Clear();
    timer.Restart();
    fsmState = State::ResetEnd;
    break;
    
  case State::ResetEnd:
    if(timer.Read() >= 500)
    {
      fsmState = State::Init;
      reset->Set();
      timer.Restart();
    }
    break;
    
  case State::Init:
   Command((uint8_t*)"+++");
    setWait(3000);
    break;
  case State::Idle:
    if(timer.Read() >= 30000)
    {
      setATCommand(ATCommand::Association);
      fsmState = State::EnterCommandMode;
      timer.Reset();
    }
    break;
  case State::EnterCommandMode:
    //initializing = false;
    Command((uint8_t*)"+++");
    setWait(2500);
    break;
  case State::ExitCommandMode:
   Command((uint8_t*)"ATCN\r\n");
    setWait(2500);
    break;
  case State::Get:
    parser->CreateGet(handler,host,route);
    Command(handler->GetPointer(0), handler->Length());
    setWait(50000);
    break;
  case State::Post:
    parser->CreatePost(handler,host,route,data,datalen);
    Command(handler->GetPointer(0), handler->Length());
    setWait(50000);
    break;
  case State::Command:
    switch(atCommand)
    {
    case ATCommand::Association:
      handler->SPrintf("%s%s%s",AT,AI,CRLF);
      Command(handler->GetPointer(0));
      break;
    case ATCommand::SleepMode:
      handler->SPrintf("%s%s 1%s",AT,SM,CRLF);
      Command(handler->GetPointer(0));
      break;
    case ATCommand::SetHost:
      handler->SPrintf("%s%s %s%s",AT,DL,(char*)host,CRLF);
      Command(handler->GetPointer(0));
      break;
    case ATCommand::SetPort:
      handler->SPrintf("%s%s %s%s",AT,DE,(char*)port,CRLF);
      Command(handler->GetPointer(0));
      break;
    case ATCommand::SetProtocol:
      handler->SPrintf("ATIP %s\r\n",(char*)protocol);
      Command(handler->GetPointer(0));
      break;
    case ATCommand::ApplyChanges:
      handler->AddArray((void*)"ATAC\r\n");
      Command(handler->GetPointer(0));
      break;
    default:
      fsmState = State::ExitCommandMode;
      break;
    }
    setWait(5000);
    break;
  case State::Wait:
    if(count++>=wait)
    {
      if(prevState == State::ExitCommandMode)
        fsmState = State::EnterCommandMode;
      else
        fsmState = prevState;
    }
    break;
  }
}
//------------------------------------------------------------------------------
bool Digi3G::Get(uint8_t* file_)
{
  if(fsmState == State::Idle)
  {
    route = file_;
    fsmState = State::Get; 
    return true;
  }
  else
    return false;
}
//------------------------------------------------------------------------------
bool Digi3G::Post(uint8_t* data_, uint16_t len)
{
  if(fsmState == State::Idle)
  {
    data = data_;
    datalen = len;
    fsmState = State::Post; 
    return true;
  }
  else
    return false;
}
//------------------------------------------------------------------------------
void Digi3G::Command(uint8_t* str)
{
  uart->WriteString(str);
  HAL::Debug((char const*)str);
}
//------------------------------------------------------------------------------
void Digi3G::Command(uint8_t* str, uint16_t len)
{
  uart->WriteString(str,len);
  HAL::Debug((char const*)str);
}
//------------------------------------------------------------------------------
void Digi3G::setWait(uint16_t w)
{
  count = 0;
  prevState = fsmState;
  wait = w;
  fsmState = State::Wait;
}
//------------------------------------------------------------------------------
void Digi3G::Parse(BufferHandler* b)
{
  //b->SwitchBuffer(handler);
  handler->CopyBuffer(b,0);
  if(fsmState != State::Idle)
  {
    uint8_t* rxb = handler->GetPointer(0);
    
    switch(prevState)
    {
    case State::Init:
      if(rxb[0] == 'O' && rxb[1] == 'K')
      {
        setATCommand(ATCommand::SetHost);
      }
      else
        fsmState = State::Init;
      break;
    case State::EnterCommandMode:
      if(rxb[0] == 'O' && rxb[1] == 'K')
        fsmState = State::Command;
      else
        fsmState = State::EnterCommandMode;
      break;
    case State::ExitCommandMode:
      if(rxb[0] == 'O' && rxb[1] == 'K')
      {
        fsmState = State::Idle;
        timer.Restart();
      }
      else
        fsmState = State::ExitCommandMode;
      break;
    case State::Command:
      if(rxb[0] == 'O' && rxb[1] == 'K')
      {
        switch(atCommand)
        {
        case ATCommand::SetHost:
          setATCommand(ATCommand::SetPort);
          break;
        case ATCommand::SetPort:
          setATCommand(ATCommand::SetProtocol);
          break;
        case ATCommand::SetProtocol:
          setATCommand(ATCommand::ApplyChanges);
          break;
        case ATCommand::ApplyChanges:
          setATCommand(ATCommand::SleepMode);
          parser->Callback()->Set(Set::ModemInitOk);
          break;
        case ATCommand::SleepMode:
          setATCommand(ATCommand::Association);
          break;
        }
      }
      else
      {
        switch(atCommand)
        {
        case ATCommand::Association:
          if(rxb[0] == '0')
            associated = true;
          else
            associated = false;
          fsmState = State::ExitCommandMode;
          atCommand = ATCommand::None;
          break;
        default:
          fsmState = State::Init;
          atCommand = ATCommand::None;
          break;
        }
      }
      break;
    case State::Post:
    case State::Get:
      // Copy RxBuffer to Local Buffer
      parser->ParseHeader(handler,true);
      fsmState = State::Idle;
      break;
    default:
      break;
    }
  }
}
//------------------------------------------------------------------------------

