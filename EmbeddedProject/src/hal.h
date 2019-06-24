/*
  FILE: hal.h
  Created on: 8/12/2017, by Tom Diaz
*/

#ifndef HAL_H
#define HAL_H

#include <constants.h>
#include <stopwatch.h>
#include <gpio.h>
#include <gpioex.h>
#include <buffer.h>
#include <uart.h>
#include <comhandler.h>
#include <unixtime.h>
#include <spw04sx.h>
#include <httpparser.h>
#include <stm32rtc.h>
#include <timehandler.h>
#include <motionControl.h>
#include <sysl476xx.h>

class ConnectionSource // For UART2 which is connected to STLink & MAX322
{
public:
  enum Value
  {
    None = 0,
    RS232 = 1,
    STLink = 2,
  };
};

class HAL
{
public:
  HAL(){};
  ~HAL(){};
  static void Init(void* sys);
  static uint32_t SysClk();
  static uint32_t OneMilliSecondSnapshot();
  static void SwitchU2Tx(ConnectionSource::Value source);
  static uint32_t CRC32(void* data, uint32_t len, uint32_t firstcrc);
  static void UartISR(uint8_t idx);
  static void Debug(const char* format, ...);

protected:
  static void Clock();
  static void Uart();
  static void I2c();
  static void Timers();
private:
};


#endif /* HAL_H */
