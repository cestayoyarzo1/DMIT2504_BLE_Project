/*
 * comport.h
 *
 *  Created on: Sep 15, 2016
 *      Author: Tomas Diaz
 */

#ifndef COMPORT_H_
#define COMPORT_H_

#include <buffer.h>
#include <stopwatch.h>
#include <constants.h>  // Defined per project.
#include <type_traits>

class CommandMode
{
public:
  enum Value
  {
    NewLine,
    Timeout,
    HwTimeout,
  };
};

class x_COMPort
{
public:
  virtual void WriteByte(uint8_t byte){}
  virtual void FlushTx(){};       //Override
protected:
  virtual void writeString(uint8_t* str, uint16_t length){}; // Virtual method
};

class COMPort : public x_COMPort
{
public:
  virtual void WriteByte(uint8_t byte) = 0;
  void WriteString(const char* srt);
  void WriteString(uint8_t* str, uint16_t length);
  void WriteString(uint8_t* str);
  void Printf(const char* format, ...);
  void Printf(const char* format, __Va_list l);
  void Echo();
  bool IsCmdComplete();
  virtual void FlushTx();       //Overriden
  void SetCommandMode(CommandMode::Value m);
  BufferHandler* TxB();
  BufferHandler *RxB();
  COMId::Value Id(); // Project specific
  template <typename T> void Write(T value)
  {
    static_assert(std::is_arithmetic<T>::value, "Arythmetic type required");
    uint8_t size = sizeof(T);
    for(uint8_t i = 0; i < size; i++)
      WriteByte(value>>(i*8));
  }
  template <typename T> void Write(T value, uint8_t c)
  {
    Write<T>(value);
    WriteByte(c);
  }
protected:
  virtual void writeString(uint8_t* str, uint16_t length) = 0; // Virtual method
  CommandMode::Value mode;
  bool cmdComplete;
  Stopwatch tOut;
  COMId::Value id;
  uint8_t* rxbuffer;
  BufferHandler* RxBuffer;
  BufferHandler* TxBuffer;
};

class DummyComPort : public COMPort
{
public:
  DummyComPort(){};
  ~DummyComPort(){};
  void WriteByte(uint8_t byte){}
  void writeString(uint8_t* str, uint16_t length){} // Virtual method
protected:
private:
};

#endif /* COMPORT_H_ */
