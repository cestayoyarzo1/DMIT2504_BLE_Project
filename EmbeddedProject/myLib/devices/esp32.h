/*
  FILE: esp32.h
  Created on: 3/24/2018, by Tom Diaz
*/
#ifndef ESP32_H
#define ESP32_H

#include <x_iot.h>
#include <uart.h>
#include <x_gpio.h>
#include <stopwatch.h>
#include <buffer.h>

union IPAddress
{
  uint32_t address;
  uint8_t bytes[4];
};

struct CommandSet
{
  uint16_t max;
  uint8_t stateWhenDone;
  const char** cmds;
};

class ESP32Server
{
public:
  enum class State
  {
    ResetBegin,
    ResetEnd,
    FlushingCommands,
    Command,
    WaitForReady,
    Idle,
    WaitState,
    WaitForAnswer
  } prevState, state, nextState;
  ESP32Server(){};
  ~ESP32Server(){};
  void Init(UART* port_, x_GPIO* reset_, BufferHandler* b);
  void FSM();
  void Parse(BufferHandler* b);
protected:
  void SetWait(State afterWait, uint32_t time);
  void SetState(State st, State nxSt);
  void SetCommand(const char* command);
  UART* port;
  x_GPIO* reset;
  const char* cmd;
  BufferHandler* buffer;
  Stopwatch timer;
  uint16_t cmdIdx;
  const char* cmdPending;
  const CommandSet* commands;
  IPAddress ip;
  bool associated;
  uint32_t lastError;
  uint32_t wait;
private:
};

#endif /* ESP32_H */
