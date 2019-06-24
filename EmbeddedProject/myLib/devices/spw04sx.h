/*
  FILE: spw04sa.h
  Created on: 2/27/2018, by Tom Diaz
*/

#include "uart.h"
#include "x_gpio.h"
#include "stopwatch.h"
#include "buffer.h"
#include "x_iot.h"

#ifndef SPW04Sx_H
#define SPW04Sx_H

#define MAX_AP_LEN 33
#define MAX_PASS_LEN 65

struct APData
{
  uint8_t AP[MAX_AP_LEN];
  uint8_t Pass[MAX_PASS_LEN];
};

class SPW04Sx : public IOTGateway
{
public:
  SPW04Sx();
  ~SPW04Sx(){};
  enum class MainFsmState
  {
    Reset,
    Init,
    WaitingForConsole,
    WaitingForAssociation,
    Idle,
    TurnOn,
    SendCommand,
    SendData,
    PostOpenSocket,
    PostWaitingForSocket,
    PostSendData,
    PostWaitForAnswer,
    PostAnswerOngoing,
    PostReadingAnswer,
    PostCloseSocket,
    Disconnect,
    HoldAfterError,
    Disabled,
    PoweringUp,
    PoweringDown,
    Configuring,
    WaitingAfterDisconnect,
    WaitForAnswer,
  };
  void Init(UART* p, x_GPIO* g, BufferHandler* b, HttpParser* parser_);
  void Parse(BufferHandler* b);
  bool Post(uint8_t* data_, uint16_t len);
  void Connect(uint8_t* address, uint16_t portNum, uint8_t protocol_);
  void Connect();
  void SendData(uint8_t* data, uint16_t len);
  void SetHost(uint8_t* host_, uint8_t* route_, uint16_t port_, uint8_t protocol_);
  void Disconnect();
  void SetAP(APData* apData_);
  void FSM();
  void Reset();
  void Enable(bool state);
  bool IsEnabled();
protected:
  void parseWind(uint16_t w);
  void wind55();
  int16_t getSocket();
  MainFsmState mainFsmState, thereAfterState;
  int16_t wind;
  int16_t lastError;
  x_GPIO* reset;
  BufferHandler* buffer;
  uint8_t* host;
  uint8_t* route;
  uint16_t port;
  uint8_t protocol;
  Stopwatch timer;  
  UART* uart;
  uint16_t bytes;
  int16_t socket;
  int16_t lSocket;
  bool wifiOn;
  uint8_t* data;
  uint16_t datalen;
  const char* cmd;
  bool powerDownRequested;
  bool configured;
  bool configuring;
  uint16_t confIdx;
  APData* apData;
  uint32_t n76Errors;
private:
};

#endif /* SPW04Sx_H */
