/*
FILE: mpack.h
Created on: 9/12/2017, by Tom Diaz
*/
#ifndef MPACK_H
#define MPACK_H

#include <cmp.h>

class MessagePack
{
public:
  MessagePack(){};
  ~MessagePack(){};
  void Init(uint8_t* buff, uint32_t len);
  void Init(void* handler_);
  void Rewind();
  bool WriteString(const char* str);
  bool WriteString(uint8_t* arr, uint32_t len);
  bool WriteInteger(int32_t val);
  bool ReadInteger(int32_t* val);
  bool WriteUInteger(uint32_t val);
  bool WriteDecimal(double val);
  bool WriteBinary(void* data, uint32_t size);
  bool WriteArray(uint32_t size);
  bool WriteMap(uint32_t size);
  bool ReadStr(uint8_t* str, uint8_t bSize);
  bool ReadObject(uint8_t* data, int32_t* size);
  bool ReadArray(uint32_t* size);
  bool ReadMap();
  bool Skip();
  bool Valid() { return lastAns; }
  uint8_t* GetBuffer();
  uint32_t GetSize();
  uint32_t objSize;
protected:
  cmp_ctx_s ctx;
  //uint32_t bufferLength;
  //uint8_t* buffer;
  //void* handler;
  //bool lastAns;
  bool lastAns;
private:
};

#endif /* MPACK_H */
