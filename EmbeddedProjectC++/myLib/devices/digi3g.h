/*
  FILE: digi3g.h
  Created on: 9/7/2017, by Tom Diaz
*/
#ifndef DIGI3G_H
#define DIGI3G_H

#include <uart.h>
#include <stopwatch.h>
//#include <httpparser.h>
#include <x_gpio.h>
#include <x_iot.h>

class Digi3G :  public IOTGateway
{
public:
  Digi3G(){};
  ~Digi3G(){};
  enum class ATCommand
  {
    None,
    SetHost,
    SetPort,
    SetProtocol,
    GetConnectionStatus,
    ApplyChanges,
    Association,
    SleepMode,
  };
  void Init(UART* uart_, x_GPIO* reset_, BufferHandler* handler_, HttpParser* parser_);
  void FSM();
  void Parse(BufferHandler* b);
  void Command(uint8_t* str);
  void Command(uint8_t* str, uint16_t len);
  void SetHost(uint8_t* host_,uint8_t* route_, uint16_t port_, uint8_t protocol_);
  bool Post(uint8_t* data_, uint16_t len);
  bool Get(uint8_t* file_);
protected:
  enum class State
  {
    ResetBegin,
    ResetEnd,
    Init,
    Idle,
    EnterCommandMode,
    ExitCommandMode,
    Get,
    Post,
    Command,
    Wait
  } fsmState, prevState;
  BufferHandler* handler;
  void makePortStr(uint16_t port_);
  void setATCommand(ATCommand cmd);
  ATCommand atCommand;
  void setWait(uint16_t w);
  uint8_t* host;
  uint8_t* route;
  uint8_t port[5];
  uint8_t protocol[2];
  uint8_t* data;
  uint16_t datalen;
  //HttpParser* parser;
  UART* uart;
  uint16_t count;
  uint16_t wait;
  Stopwatch timer;
  x_GPIO* reset;
private:
};


#endif /* DIGI3G_H */
