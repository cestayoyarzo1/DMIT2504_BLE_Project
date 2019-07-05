/*
  FILE: spw04sx.cpp
  Created on: 2/27/2018, by Tom Diaz
*/

#include "spw04sx.h"
#include "hal.h"
#include "stdio.h"

#define WIND_COMMAND_COLON_LOC 5
#define WIND55_SOCKET_LOC 10
#define ATS_ANS 5
#define RESET_PULSE_WIDTH
#define ASSOCIATION_TIMEOUT 60000 * 60 * 24//60000
#define CONSOLE_TIMEOUT 20000
#define HOLD_AFTER_ERROR_WINDOW 5000
#define REPLY_TIMEOUT 10000
#define REPLY_CLOSING_WINDOW 1000
#define CONFIG_COMMANDS 3

static const char* AT = "AT+S.";
static const char* SOCKR = "SOCKR=";
static const char* SOCKON = "SOCKON=";
static const char* SOCKC = "SOCKC=";
static const char* SOCKW = "SOCKW=";

static const char* configuration[]
{
  "AT+S.SCFG=console_echo,0\r\nAT+S.SCFG=console_winds,1\r\nAT+S.SCFG=console_errs,1\r\n",
  "AT+S.SCFG=wifi_auth_type,0\r\n""AT+S.SCFG=wifi_priv_mode,2\r\n",
};

SPW04Sx::SPW04Sx()
{
  wind = -1;
  lastError = -1;
  socket = -1;
  wifiOn = false;
  associated = false;
  thereAfterState = MainFsmState::Idle;
}
//------------------------------------------------------------------------------
void SPW04Sx::SetAP(APData* apData_)
{
  apData = apData_;
  configured = false;
  mainFsmState = MainFsmState::Reset;
}
//------------------------------------------------------------------------------
void SPW04Sx::Init(UART* p, x_GPIO* g, BufferHandler* b, HttpParser* parser_)
{
  reset = g;
  buffer = b;
  uart = p;
  parser = parser_;
  Reset();
}
//------------------------------------------------------------------------------
void SPW04Sx::SetHost(uint8_t *host_, uint8_t *route_, uint16_t port_, uint8_t protocol_)
{
  host = host_;
  route = route_;
  port = port_;
  protocol = protocol_;
}
//------------------------------------------------------------------------------
bool SPW04Sx::Post(uint8_t* data_, uint16_t len)
{
  if((mainFsmState == MainFsmState::Idle) && associated)
  {
    data = data_;
    datalen = len;
    mainFsmState = MainFsmState::PostOpenSocket;
    return true;
  }
  else
    return false;
}
//------------------------------------------------------------------------------
void SPW04Sx::Connect(uint8_t *address, uint16_t portNum, uint8_t protocol_)
{
  if(socket == -1)      // No active connection
  {
    if(protocol_ == 1)
      sprintf((char*)uart->TxB()->GetPointer(0),"%s%s%s,%u,,t\r",AT,SOCKON,address,portNum);
    else
      sprintf((char*)uart->TxB()->GetPointer(0),"%s%s%s,%u,,%s\r",AT,SOCKON,address,portNum,address);
    uart->TxB()->Adjust();
    mainFsmState = MainFsmState::SendCommand;
  }
}
//------------------------------------------------------------------------------
void SPW04Sx::Connect()
{
  if(socket == -1)      // No active connection
  {
    sprintf((char*)uart->TxB()->GetPointer(0),"%s%s%s,%u,,t\r",AT,SOCKON,host,port);
    uart->TxB()->Adjust();
    mainFsmState = MainFsmState::SendCommand;
  }
}
//------------------------------------------------------------------------------
void SPW04Sx::Disconnect()
{
  if(socket > -1)
  {
    sprintf((char*)uart->TxB()->GetPointer(0),"%s%s%u\r",AT,SOCKC,socket);
    uart->TxB()->Adjust();
    mainFsmState = MainFsmState::SendCommand;
  }
  else
    mainFsmState = MainFsmState::WaitingAfterDisconnect;
}
//------------------------------------------------------------------------------
void SPW04Sx::SendData(uint8_t *data, uint16_t len)
{
  if(socket > -1)
  {
    uart->TxB()->Clear();
    sprintf((char*)uart->TxB()->GetPointer(0),"%s%s%u,%u\r",AT,SOCKW,socket,len);
    uart->TxB()->Adjust();
    //HAL::Debug("%s\n",uart->TxB()->GetPointer(0));
    uart->TxB()->AddArray(data,len);
    mainFsmState = MainFsmState::SendData;
  }
}
//------------------------------------------------------------------------------
void SPW04Sx::Enable(bool state)
{
  //mainFsmState = state ? MainFsmState::Reset : MainFsmState::Disabled;
  if(state)
  {
    uart->TxB()->Clear();
    uart->TxB()->AddArray((char*)"AT+S.WIFI=1\r\n");
    thereAfterState = MainFsmState::PoweringUp;
    mainFsmState = MainFsmState::SendData;
  }
  else
  {
    uart->TxB()->Clear();
    uart->TxB()->AddArray((char*)"AT+S.WIFI=0\r\n");
    thereAfterState = MainFsmState::PoweringDown;
    mainFsmState = MainFsmState::SendData;
    powerDownRequested = true;
  }
}
//------------------------------------------------------------------------------
bool SPW04Sx::IsEnabled()
{
  return mainFsmState == MainFsmState::Disabled;
}
//------------------------------------------------------------------------------
void SPW04Sx::FSM()
{
  switch(mainFsmState)
  {
  case MainFsmState::Disabled:
    break;
    
  case MainFsmState::Reset:
    confIdx = 0;
    associated = false;
    socket = -1;
    reset->Clear(); // Init unit in reset state
    timer.StopAndReset();
    timer.Start();
    mainFsmState = MainFsmState::Init;
    break;
    
  case MainFsmState::Init:
    if(timer.Read()>=500)
    {
      reset->Set(); // Enable Unit
      mainFsmState = MainFsmState::WaitingForConsole;
      timer.Reset();
    }
    break;
    
  case MainFsmState::HoldAfterError:
    if(timer.Read() >= HOLD_AFTER_ERROR_WINDOW)
    {
      mainFsmState = thereAfterState;
      timer.StopAndReset();
    }
    break;
    
  case MainFsmState::WaitingForConsole:
    // Waiting for WIND 0 Async Message
    if(timer.Read() >= CONSOLE_TIMEOUT)
    {
      mainFsmState = MainFsmState::Reset;
      timer.StopAndReset();
    }
    break;
    
  case MainFsmState::WaitingForAssociation:
    // Waiting for WIND 24 Async Message
    if(timer.Read() >= ASSOCIATION_TIMEOUT)
    {
      mainFsmState = MainFsmState::Reset;
      timer.StopAndReset();
    }
    break;
    
  case MainFsmState::Idle:
    // Console is ready, waiting for commands
    break;
    
  case MainFsmState::TurnOn:
    break;
    
  case MainFsmState::PostOpenSocket:
    Connect();
    thereAfterState = MainFsmState::PostWaitingForSocket;
    break;
    
  case MainFsmState::PostWaitingForSocket:
    if(socket > -1)
    {
      // Post
      parser->CreatePost(buffer,host,route,data,datalen);
      SendData(buffer->GetPointer(0),buffer->Length());
      timer.StopAndReset();
      timer.Start();
      thereAfterState = MainFsmState::PostWaitForAnswer;
    }
    break;
    
  case MainFsmState::PostWaitForAnswer:
    if(timer.Read() >= REPLY_TIMEOUT)
    {
      mainFsmState = MainFsmState::Disconnect;
      timer.StopAndReset();
    }
    break;
    
  case MainFsmState::PostAnswerOngoing:
    if(timer.Read() >= REPLY_CLOSING_WINDOW)
    {
      // Predefined time window after last WIND55, lets close reply msg.
      HAL::Debug("%s%s%u,%u\n",AT,SOCKR,socket,bytes);
      sprintf((char*)uart->TxB()->GetPointer(0),"%s%s%u,%u\r",AT,SOCKR,socket > -1 ? socket : lSocket, bytes);
      uart->TxB()->Adjust();
      mainFsmState = MainFsmState::SendCommand;
      thereAfterState = MainFsmState::PostReadingAnswer;
      timer.StopAndReset();
    }
    break;
    
  case MainFsmState::PostReadingAnswer:
    if(socket == -1)
      mainFsmState = MainFsmState::Disconnect;
    break;
    
  case MainFsmState::Disconnect:
    Disconnect();
    thereAfterState = MainFsmState::WaitingAfterDisconnect;
    break;
    
  case MainFsmState::WaitingAfterDisconnect:
    if(socket == -1)
      mainFsmState = MainFsmState::Idle;
    break;
    
  case MainFsmState::SendCommand:
    //HAL::Debug("%s\n",uart->TxB()->GetPointer(0));
    uart->FlushTx();
    mainFsmState = thereAfterState;
    break;
    
  case MainFsmState::SendData:
    //HAL::Debug("Sending %u bytes\n",uart->TxB()->Length());
    //HAL::Debug("%s",uart->TxB()->GetPointer(0));
    uart->FlushTx();
    mainFsmState = thereAfterState;
    break;
    
  case MainFsmState::Configuring:
    if(confIdx < CONFIG_COMMANDS)
    {
      if(confIdx < (CONFIG_COMMANDS - 1))
      {
        HAL::Debug(configuration[confIdx]);
        uart->TxB()->Clear();
        uart->TxB()->AddArray((void*)configuration[confIdx++]);
        thereAfterState = MainFsmState::WaitForAnswer;
        mainFsmState = MainFsmState::SendData;
      }
      else
      {
        uart->TxB()->Clear();
        sprintf((char*)uart->TxB()->GetPointer(0),
                "AT+S.SSIDTXT=%s\r\nAT+S.SCFG=wifi_wpa_psk_text,%s\r\n",
                apData->AP,apData->Pass);
        uart->TxB()->Adjust();
        HAL::Debug("%s",uart->TxB()->GetPointer(0));
        ++confIdx;
        thereAfterState = MainFsmState::WaitForAnswer;
        mainFsmState = MainFsmState::SendData;
      }
    }
    else
    {
      configuring = false;
      configured = true;
      Enable(true);
    }
    break;
    
  case MainFsmState::WaitForAnswer:
    break;
  }
}
//------------------------------------------------------------------------------
void SPW04Sx::Reset()
{
  mainFsmState = MainFsmState::Reset;
  //HAL::Debug("SPW04Sx Reset\n");
}
//------------------------------------------------------------------------------
void SPW04Sx::parseWind(uint16_t w)
{
  switch(w)
  {
  case 0:       // Console is ON
    timer.Reset();
    HAL::Debug("Wifi Console On\r\n");
    mainFsmState = MainFsmState::Idle;
    break;
  case 24:      // Module is Associated
    timer.StopAndReset();
    mainFsmState = MainFsmState::Idle;
    associated = true;
    break;
  case 32:      // Wifi Radio is ON
    if(!configured)
    {
      uart->TxB()->Clear();
      uart->TxB()->AddArray((char*)"AT+S.WIFI=0\r\n");
      thereAfterState = MainFsmState::PoweringDown;
      mainFsmState = MainFsmState::SendData;
    }
    else
    {
      wifiOn = true;
      mainFsmState = MainFsmState::WaitingForAssociation;
    }
    break;
  case 38:      // Wifi Radio is OFF
    wifiOn = false;
    associated = false;
    if(mainFsmState == MainFsmState::PoweringDown)
    {
      associated = false;
      if(!configured)
      {
        mainFsmState = MainFsmState::Configuring;
        configuring = true;
      }
      else
        mainFsmState = MainFsmState::Disabled;
      powerDownRequested = false;
      HAL::Debug("Wifi powered off\r\n");
    }
    break;
  case 30:
  case 33:
  case 49:      // Module is not associated anymore
    timer.Start();
    if(mainFsmState != MainFsmState::PoweringDown)
    {
      mainFsmState = MainFsmState::Reset;
    }
    associated = false;
    break;
  case 55:      // Socket Received data
    wind55();
    break;
  case 58:      // Socket disconnected
    lSocket = socket;
    socket = -1;
    break;
  }
  wind = -1;
}
//------------------------------------------------------------------------------
void SPW04Sx::wind55() // Retrieve data in socket
{
  char* end;

  switch(mainFsmState)
  {
  case MainFsmState::PostWaitForAnswer:
    // Get number of bytes in wifi buffer and go to PostAnswerOngoing
    socket = strtol(cmd,&end,10);
    cmd = strchr(end + 1,':') + 1;
    bytes = strtol(cmd,&end,10);
    mainFsmState = MainFsmState::PostAnswerOngoing;
    timer.Reset();
    break;
    
  case MainFsmState::PostAnswerOngoing:
    // If we get another WIND55 extract number of bytes and restar timeout
    socket = strtol(cmd,&end,10);
    cmd = strchr(end + 1,':') + 1;
    bytes = strtol(cmd,&end,10);
    timer.Reset();
    break;
    
  default:
    socket = strtol(cmd,&end,10);
    cmd = strchr(end + 1,':') + 1;
    bytes = strtol(cmd,&end,10);
    //HAL::Debug("%s%s%u,%u\r",AT,SOCKR,socket,bytes);
    sprintf((char*)uart->TxB()->GetPointer(0),"%s%s%u,%u\r",AT,SOCKR,socket,bytes);
    uart->TxB()->Adjust();
    mainFsmState = MainFsmState::SendCommand;
    thereAfterState = MainFsmState::Idle;
    break;
  }
}
//------------------------------------------------------------------------------
int16_t SPW04Sx::getSocket()
{
  int16_t s = -1;
  //const char* cmd = (const char*)buffer->GetPointer(8);
  char* end = 0;
  
  cmd = strchr(cmd,':') + 1;
  s = (int16_t)strtol(cmd,&end,10);
  
  return s;
}
//------------------------------------------------------------------------------
void SPW04Sx::Parse(BufferHandler* b)
{
  if(mainFsmState == MainFsmState::Disabled)
    return;
    
  buffer->CopyBuffer(b,0);
  char* end;
  cmd = (const char*)buffer->GetPointer(0);
  
  while(cmd && cmd[0]) // cmd is not NULL  nad first char is not terminator
  {
    if(!strncmp(cmd,"+WIND",5)) // "WIND" Async msg
    {
      //+WIND:55::socket::bytes:totalBytes
      // Advance pointer to location after colon "+WIND: ", where code is
      cmd = strchr(cmd,':') + 1;
      //wind = strtol(cmd + WIND_COMMAND_COLON_LOC + 1,&end,10); // Get Wind ID
      wind = strtol(cmd,&end,10); // Get Wind ID
      // Advance pointer 2 locations, so only useful information after :: is left
      cmd = end + 2;
      // HAL::Debug("WIND %u\n", wind);
      // React to wind ID method
      parseWind(wind);
    }
    else if(!strncmp(cmd,"AT-S",4))
    {
      //HAL::Debug("%s\n",buffer->GetPointer(0));
      // Advance cmd pointer to answer string
      cmd = strchr(cmd,'.') + 1; // ie. AT-S.[Answer string]:[Data]
      
      if(!strncmp(cmd,"OK",2))
      //if(cmd[ATS_ANS] == 'O' && cmd[ATS_ANS+1] == 'K')        // OK
      {
        if(configuring)
          mainFsmState = MainFsmState::Configuring;
        //HAL::Debug("OK\n");
      }
      else if(!strncmp(cmd,"ERROR",5))
      //else if(cmd[ATS_ANS] == 'E' && cmd[ATS_ANS+1] == 'R') // ERROR
      {
        // Advance cmd pointer to error code
        cmd = strchr(cmd,':') + 1; // ie. AT-S.ERROR:[Code]
        lastError = strtol(cmd,&end,10); // Get Error ID
        mainFsmState = MainFsmState::HoldAfterError;
        timer.Start();
        switch(lastError)
        {
        case 76:
          ++n76Errors;
          break;
        case 74:
          thereAfterState = MainFsmState::Idle;
          HAL::Debug("Socket connection timeout: %u\n", lastError);
          parser->SetResponse(500); // Force an Internal Server Error code.
          break;
        case 44:
          thereAfterState = MainFsmState::Reset;
          HAL::Debug("Tried to open connection without association: %u\n", lastError);
          parser->SetResponse(500); // Force an Internal Server Error code.
          break;
        default:
          thereAfterState = MainFsmState::Reset;
          HAL::Debug("ERROR: %u\n", lastError);
          break;
        }
        // after "ERROR" there is a colon ':' and then a numeric code
        HAL::Debug("W_ERROR %u\n", lastError);
      }
      else if(!strncmp(cmd,"Read",4))
      //else if(cmd[ATS_ANS] == 'R' && cmd[ATS_ANS+1] == 'e')
      {
        // Reading of packet in buffer
        // Advance pointer to the location after AT-S.Read:
        // We need to get rid of the "AT-S.Read" string located in the first line
        // so we can pass the buffer to the HttpParse.
        uint8_t* newLine = (uint8_t*)strchr((const char*)cmd,'\n') + 1;
        // Discard Wifi Command, probably will need to create buffer->DiscardFromEnd
        buffer->Discard(newLine - buffer->GetPointer(0));
        parser->ParseHeader(buffer,true);
        //HAL::Debug("%s",buffer->GetPointer(0));
        bytes = 0;
        mainFsmState = MainFsmState::Disconnect;
      }
      else if(!strncmp(cmd,"On",2))
      //else if(cmd[ATS_ANS] == 'O' && cmd[ATS_ANS+1] == 'n')
      {
        // Advance pointer to the location after AT-S.On:
        cmd = strchr(cmd,':') + 1;
        // Socket Connected
        //HAL::Debug("Socket Connected\n");
        socket = getSocket();
      }
    }
    // Look for next line end
    cmd = strchr(cmd,'\n');
    // If cmd is not null then cmd+=1 to find next line start
    cmd += cmd ? 1 : 0;
  }
  buffer->Clear();
}
