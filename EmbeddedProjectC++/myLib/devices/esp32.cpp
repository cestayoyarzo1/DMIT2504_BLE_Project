/*
  FILE: esp32.cpp
  Created on: 3/24/2018, by Tom Diaz
*/

#include <esp32.h>
#include <string.h>

#define RESET_WINDOW 500

const char* GetIP = "AT+CIFSR\r\n";

const char* InitCommands[] = 
{
  "ATE0\r\n",
  //"AT+SLEEP=1\r\n",
  //"AT+CWMODE=2\r\n"
  "AT+CIPMUX=1\r\n",
  "AT+CIPSERVER=1,333\r\n",
  "AT+CIPSTO=5\r\n"
};

//const CommandSet InitCommandSet = { 1, (uint8_t)ESP32Server::State::Idle, (char const**)"ATE0\r\n" };

// Command array length, N=next state after flushing commands, command array
const CommandSet defaultSet = { 0 , (uint8_t)ESP32Server::State::Idle, 0 };
const CommandSet InitCommandSet = { 4, (uint8_t)ESP32Server::State::Idle,InitCommands };
//const CommandSet GetIp = {1, (uint8_t)ESP32Server::State::Idle, (char const**)"AT+CIFSR\r\n"};

void ESP32Server::Init(UART* port_, x_GPIO* reset_, BufferHandler* b)
{
  port = port_;
  reset = reset_;
  buffer = b;
}
//------------------------------------------------------------------------------
void ESP32Server::FSM()
{
  switch(state)
  {
  case State::ResetBegin:
    reset->Clear();
    state = State::ResetEnd;
    timer.Start();
    break;
    
  case State::ResetEnd:
    if(timer.Read() >= RESET_WINDOW)
    {
      reset->Set();
      state = State::WaitForReady;
      timer.StopAndReset();
    }
    break;
    
  case State::Command:
    // This could be changed to Printf and add AT+ and \r\n in real time, less
    // flash used but more processing power (battery) usage maybe?
    port->WriteString(cmdPending);
    state = nextState;
    break;
    
  case State::WaitForReady:
    break;
    
  case State::FlushingCommands:
    SetCommand(InitCommands[cmdIdx]);
    break;
    
  case State::Idle:
    break;
    
  case State::WaitState:
    if(timer.Read() >= wait)
    {
      SetState(nextState, State::WaitForAnswer);
      timer.Stop();
    }
    break;
    
  case State::WaitForAnswer:
    break;
  }
}
//------------------------------------------------------------------------------
void ESP32Server::SetCommand(const char* command)
{
  cmdPending = command;
  SetState(State::Command, State::WaitForAnswer);
}
//------------------------------------------------------------------------------
void ESP32Server::SetState(State st, State nxSt)
{
  prevState = state;
  state = st;
  nextState = nxSt;
}
//------------------------------------------------------------------------------
void ESP32Server::SetWait(ESP32Server::State afterWait, uint32_t time)
{
  wait = time;
  timer.StopAndReset();
  SetState(State::WaitState, afterWait);
  timer.Start();
}
//------------------------------------------------------------------------------
uint32_t ans = 0;
void ESP32Server::Parse(BufferHandler* b)
{
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
      cmdIdx = 0;
      commands = &InitCommandSet;
      SetState(State::FlushingCommands,State::FlushingCommands);
    }
    else if(!strncmp("OK",cmd,2))
    {
      if(commands->max > 0)
      {
        ++cmdIdx;
        if(cmdIdx < commands->max)
          SetState(State::FlushingCommands, State::Command);
        else
        {
          cmdIdx = 0;
          commands = &defaultSet;
          SetState(State::Idle, State::Idle);
        }
      }
    }
  }
  else if(!strncmp("WIFI",cmd,4))       // WIFI connection message
  {
    cmd = cmd + 5; // Get rid of "WIFI " string
    if(!strncmp("GOT IP",cmd,3))
    {
      // Query IP
      cmdPending = GetIP;
      SetWait(State::Command, 1000);
      //SetState(State::Command, State::WaitForAnswer);
    }
    else if(!strncmp("CONNECTED",cmd,3))
    {
      associated = true;
    }
    else        // Disconnected
    {
      ip.address = 0; // Clear ip address
      associated = false;
    }
  }
  else if(cmd[0] == '+')
  {
    cmd = cmd + 1; // Get rid of "+" string
    if(!strncmp("CIFSR",cmd,5))
    {
      cmd = cmd + 13; // Get rid of " +CIFSR:STAIP," " string
      char* endptr = 0;
      for(uint32_t i = 0; i < 4; i++)
      {
        ip.bytes[i] = strtol(cmd, &endptr, 10);
        cmd = endptr + 1;
      }
    }
    SetState(State::Idle, State::WaitForAnswer);
  }
  else if(!strncmp("ERR",cmd,3))
  {
    cmd = cmd + 9; // Get rid of "ERR CODE:" " string
    char* endptr = 0;
    lastError = strtol(cmd, &endptr, 16);
  }
}