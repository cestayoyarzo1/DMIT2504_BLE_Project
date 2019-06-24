/*
  FILE: sysl432xx.h
  Created on: 8/13/2017, by Tom Diaz
*/
#ifndef SYSL432XX_H
#define SYSL432XX_H

#include <global_constants.h>
#include <x_system.h>
#include <spidevice.h>
#include <spiport.h>

class Process
{
public:
  enum Value
  {
    Parse_COMPort_Message,
  };
};

class Get
{
public:
  enum Value
  {
    Uart2_Valid,
    Uart2_Valid_Edge,
  };
};

class Set
{
public:
  enum Value
  {
    Uart_Write_String,
    Rtc,
  };
};

class LogEntry
{
public:
  uint32_t Valid;
  uint32_t Time;
  uint32_t ID;
  uint32_t Message;
  static uint8_t buffer[128];
};

class SysL476xx : public x_System
{
public:
  SysL476xx();
  ~SysL476xx(){};
  void Init();
  void Run();
  intptr_t Process(uint32_t msg);
  intptr_t Get(uint32_t msg);
  intptr_t Set(uint32_t msg);
  intptr_t Clear(uint32_t msg);
  void parse(COMPort* p, uint16_t idx);
  GPIO led;
  GPIOEx ledEx;
  GPIO button;
  GPIOEx buttonEx;
  ButtonState buttonState;
  UART uart1;
  UART uart2;
  UART debugUart;
  COMHandler comHandler;
  STM32Rtc rtc;
  TimeHandler timeHandler;
  uint32_t time;
  
  MotionControl robot;
  SPIPort btSpi; 
  SPIDevice leBluetooth;
  GPIO btCs;
  
protected:  
  BufferHandler wifiBuffer;
  uint8_t wifiRawBuffer[COM_RX_BUFFER_SIZE];
  
  BufferHandler wifiRxBuffer;
  BufferHandler wifiTxBuffer;
  uint8_t wifiRawRxBuffer[COM_RX_BUFFER_SIZE];
  uint8_t wifiRawTxBuffer[COM_TX_BUFFER_SIZE];
  
  BufferHandler commRxBuffer;
  BufferHandler commTxBuffer;
  uint8_t commRawRxBuffer[COM_RX_BUFFER_SIZE];
  uint8_t commRawTxBuffer[COM_TX_BUFFER_SIZE];
  
  BufferHandler debugRxBuffer;
  BufferHandler debugTxBuffer;
  uint8_t debugRawRxBuffer[COM_RX_BUFFER_SIZE];
  uint8_t debugRawTxBuffer[COM_TX_BUFFER_SIZE];
};


#endif /* SYSL432XX_H */
