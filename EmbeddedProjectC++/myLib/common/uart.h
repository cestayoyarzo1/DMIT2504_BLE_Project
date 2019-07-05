/*
 * uart.h
 *
 *  Created on: Sep 15, 2016
 *      Author: Tom Diaz
 */

#ifndef UART_H_
#define UART_H_

#include <stdint.h>
#include <comport.h>

class BaudRate
{
public:
  enum Value
  {
    B_1200 = 1200,
    B_2400 = 2400,
    B_4800 = 4800,
    B_9600 = 9600,
    B_19200 = 19200,
    B_38400 = 38400,
    B_57600 = 57600,
    B_115200 = 115200,
    B_230400 = 230400,
    B_460800 = 460800,
    B_921600 = 921600,
  };
};

class UartParity
{
public:
  enum Value
  {
    None,
    Even,
    Odd,
  };
};

class UART : public COMPort
{
public:
  void Init(COMId::Value uid, BufferHandler* rx, BufferHandler* tx, 
                bool dmaEnabled_);
  void Init(COMId::Value uid, BaudRate::Value baudRate_, BufferHandler* rx, 
                BufferHandler* tx, bool dmaEnabled_);
  virtual void WriteByte(uint8_t byte); // Overriden
  void FlushTx();       //Overriden
  void AddToRx(uint8_t byte);
  void ISR();
  void SetParity(UartParity::Value p);
protected:
  virtual uint32_t getBRR(uint32_t baudRate);
  virtual void writeString(uint8_t* str, uint16_t length); // Overriden
  USART_TypeDef *uart;
  BaudRate::Value baudRate;
  bool dmaEnabled;
  UartParity::Value parity;
};

#endif /* UART_H_ */
