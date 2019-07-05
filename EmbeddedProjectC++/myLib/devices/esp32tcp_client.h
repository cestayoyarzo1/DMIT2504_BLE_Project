/*
  FILE: esp32tcp_client.h
  Created on: 5/7/2018, by Tom Diaz
*/
#ifndef ESP32TCP_CLIENT_H
#define ESP32TCP_CLIENT_H

#include <x_gpio.h>
#include <buffer.h>
#include <httpparser.h>
#include <uart.h>
#include <stopwatch.h>
#include <gpioex.h>

class Esp32TcpClient
{
public:
  Esp32TcpClient(){};
  ~Esp32TcpClient(){};
  enum class State
  {
    ResetBegin,
    ResetEnd,
    Waiting,
    OpenSocket,
    SendData,
    WaitingForReply,
    SetupTx,
    Idle,
    Disabled,
  };
  void Init(UART* p, x_GPIO* g, BufferHandler* b, HttpParser* parser_);
  void Parse(BufferHandler* b);
  bool Post(uint8_t* data_, uint16_t len_);
  void SetLedPtr(GPIOEx* ledEx_) { ledEx = ledEx_; }
  bool IsOnline();
  void Enable(bool state);
  //void Connect(uint8_t* address, uint16_t portNum, uint8_t protocol_);
  //void Connect();
  //void SendData(uint8_t* data, uint16_t len);
  void SetHost(uint8_t* host_, uint8_t* route_, uint16_t port_, uint8_t protocol_);
  void SetAP(uint8_t* apn_, uint8_t* pass_);
  void FSM();
protected:
  void ledPattern(LedPattern::Value p);
  enum class CommandId
  {
    None,
    EchoOff,
    DisableAutoConnect,
    SetMode,
    ConnectToAP,
    OpenSocket,
    SetupTx,
  };
  UART* uart;
  x_GPIO* reset;
  GPIOEx* ledEx;
  BufferHandler* buffer;
  HttpParser* parser;
  State fsmState;
  Stopwatch timer;
  char const* cmd;
  bool associated;
  bool isSocketOpen;
  uint8_t* apn;
  uint8_t* pass;
  uint8_t* data;
  uint16_t len;
  uint8_t* host;
  uint8_t* route;
  uint16_t port;
  uint8_t protocol;
  //uint8_t commandBuffer[64];
  CommandId lastCommand;
  Stopwatch socketTTL;
private:
};

#endif /* ESP32TCP_CLIENT_H */
