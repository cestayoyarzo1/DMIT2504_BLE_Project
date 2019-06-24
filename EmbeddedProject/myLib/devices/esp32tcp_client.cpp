/*
  FILE: esp32tcp_client.cpp
  Created on: 5/7/2018, by Tom Diaz
*/

#include "esp32tcp_client.h"
#include "hal.h"
#include "stdio.h"

#define SOCKET_TTL 4000
#define REPLY_TIMEOUT 3000

void Esp32TcpClient::Init(UART* p, x_GPIO* g, BufferHandler* b, HttpParser* parser_)
{
  uart = p;
  reset = g;
  buffer = b;
  parser = parser_;
}
//------------------------------------------------------------------------------
void Esp32TcpClient::FSM()
{
  switch(fsmState)
  {
  case State::ResetBegin:
    HAL::Debug("ESP32 Init...\r\n");
    reset->Clear();
    timer.Restart();
    fsmState = State::ResetEnd;
    ledPattern(LedPattern::Toggle_1s);
    break;
    
  case State::ResetEnd:
    if(timer.Read() >= 500)
    {
      reset->Set();
      timer.Restart();
      fsmState = State::Waiting;
    }
    break;
    
  case State::OpenSocket:
    uart->Printf("AT+CIPSTART=\"TCP\",\"%s\",%d\r\n",host,port);
    fsmState = State::Waiting;
    break;
    
  case State::SetupTx:
    socketTTL.Reset();
    parser->CreatePost(buffer,host,route,data,len);
    lastCommand = CommandId::SetupTx;
    fsmState = State::Waiting;
    uart->Printf("AT+CIPSEND=%d\r\n",buffer->Length());
    break;
    
  case State::SendData:
    parser->CreatePost(buffer,host,route,data,len);
    uart->WriteString(buffer->GetPointer(0));
    fsmState = State::WaitingForReply;
    timer.Restart();
    len = 0;
    break;
    
  case State::WaitingForReply:
    if(timer.Read() >= REPLY_TIMEOUT)
    {
      fsmState = State::ResetBegin;
    }
    break;
    
  case State::Idle:
    break;
    
  case State::Waiting:
    break;
    
  default:
    break;
  }
  
  if(socketTTL.Read() >= SOCKET_TTL)
  {
    uart->WriteString("AT+CIPCLOSE\r\n");
    socketTTL.Reset();
  }
}
//------------------------------------------------------------------------------
void Esp32TcpClient::Enable(bool state)
{
  if(state)
  {
    fsmState = State::ResetBegin;
  }
  else
  {
    fsmState = State::Disabled;
  }
}
//------------------------------------------------------------------------------
bool Esp32TcpClient::IsOnline()
{
  return associated;
}
//------------------------------------------------------------------------------
void Esp32TcpClient::SetHost(uint8_t *host_, uint8_t *route_, uint16_t port_, uint8_t protocol_)
{
  host = host_;
  route = route_;
  port = port_;
  protocol = protocol_;
}
//------------------------------------------------------------------------------
void Esp32TcpClient::SetAP(uint8_t *apn_, uint8_t *pass_)
{
  apn = apn_;
  pass = pass_;
  fsmState = State::ResetBegin;
}
//------------------------------------------------------------------------------
void Esp32TcpClient::ledPattern(LedPattern::Value p)
{
  if(ledEx)
  {
    ledEx->SetPattern(p);
  }
}
//------------------------------------------------------------------------------
bool Esp32TcpClient::Post(uint8_t* data_, uint16_t len_)
{
  if((fsmState == State::Idle) && associated)
  {
    socketTTL.Reset();
    data = data_;
    len = len_;
    fsmState = isSocketOpen ? State::SetupTx : State::OpenSocket;
    
    return true;
  }
  else
    return false;
}
//------------------------------------------------------------------------------
void Esp32TcpClient::Parse(BufferHandler* b)
{
  if(fsmState == State::Disabled)
    return;
  
  buffer->CopyBuffer(b,0);
  cmd = (const char*)buffer->GetPointer(0);
  
  // Stip echo off
  if(!strncmp("AT",cmd,2)) // Command echo present
  {
    // Echo format = AT...\r\n
    cmd = strchr(cmd,'\n');     // Look for that last \n
    if(cmd)                     // If found
      cmd = cmd + 1;            // Advance the pointer three positions
    else
      return;                   // No more useful information in cmd
  }
  
  if(!strncmp("\r\n",cmd,2))// New line starts as response start
  {
    cmd = cmd + 2;              // Get rid of it
    
    if(!strncmp("ready",cmd,5)) // ready string after reset
    {
      // Module is ready to receive commands
#ifdef DEBUG_UART
      uart->WriteString((uint8_t*)"ATE1\r\n");
#else
      uart->WriteString((uint8_t*)"ATE0\r\n");
#endif
      lastCommand = CommandId::EchoOff;
      fsmState = State::Waiting;
    }
    else if(!strncmp("OK",cmd,2))
    {
      // OK response to last command.
      switch(lastCommand)
      {
      case CommandId::EchoOff:
        //fsmState = State::Idle;
        uart->WriteString((uint8_t*)"AT+CWAUTOCONN=0\r\n");
        lastCommand = CommandId::DisableAutoConnect;
        fsmState = State::Waiting;
        break;
        
      case CommandId::DisableAutoConnect:
        //uart->WriteString((uint8_t*)"AT+CWAUTOCONN=0\r\n");
        uart->WriteString((uint8_t*)"AT+CWMODE=1\r\n");
        lastCommand = CommandId::SetMode;
        fsmState = State::Waiting;
        break;
        
      case CommandId::SetMode:
        uart->Printf("AT+CWJAP=\"%s\",\"%s\"\r\n",apn,pass);
        lastCommand = CommandId::ConnectToAP;
        fsmState = State::Waiting;
        break;
        
      case CommandId::ConnectToAP:
        fsmState = State::Idle;
        break;
        
      case CommandId::SetupTx:
        fsmState = State::SendData;
        break;
        
      default:
        HAL::Debug("OK response to CommandId %d", lastCommand);
        break;
      }
    }
    else if(!strncmp("+IPD",cmd,4))
    {
      socketTTL.Reset();
      parser->ParseHeader(buffer, true);
      fsmState = State::Idle;
    }
    else if(!strncmp("ERROR",cmd,5))
    {
      fsmState = State::ResetBegin;
    }
  }
  else if(!strncmp("WIFI",cmd,4))       // WIFI connection message
  {
    cmd = cmd + 5; // Get rid of "WIFI " string
    if(!strncmp("GOT IP",cmd,3))
    {
      associated = true;
      ledPattern(LedPattern::Beacon);
    }
    else if(!strncmp("CONNECTED",cmd,3))
    {
      ledPattern(LedPattern::Beacon);
    }
    else        // "WIFI DISCONNECTED"
    {
      //ip.address = 0; // Clear ip address
      associated = false;
      ledPattern(LedPattern::Toggle_1s);
    }
  }
  else if(!strncmp("CONNECT",cmd,7))       // Socket Connected
  {
    if(len > 0)
      fsmState = State::SetupTx;
    isSocketOpen = true;
    ledPattern(LedPattern::Toggle_25ms);
    socketTTL.Start();
  }
  else if(!strncmp("CLOSED",cmd,6))       // Socket Connected
  {
    isSocketOpen = false;
    socketTTL.StopAndReset();
    ledPattern(associated ? LedPattern::Beacon : LedPattern::Toggle_1s);
  }
  else if(!strncmp("ERR",cmd,3))       // Error
  {
    fsmState = State::ResetBegin;
  }
  else if(!strncmp("+CWJAP",cmd,6))    // Connection to AP failed
  {
    fsmState = State::Idle;
  }
}
