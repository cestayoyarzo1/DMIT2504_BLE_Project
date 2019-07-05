/*
 * buffer.h
 *
 *  Created on: Sep 15, 2016
 *      Author: Tom Diaz
 */

#ifndef BUFFER_H_
#define BUFFER_H_

#include <stdint.h>
#include <string.h>
#include <type_traits>
   
class BufferHandler
{
public:
  BufferHandler();
  ~BufferHandler(){};
  void Adjust();
  void Close();
  void SetLength(uint32_t len);
  void Init(uint8_t* buff,uint32_t size);
  void InsertByte(uint16_t location, uint8_t data);
  void AddArray(void* data);
  int32_t Find(uint8_t byte);
  void GetString(void* dst,uint32_t start);
  void GetArray(void* dst, uint32_t start, uint32_t len);
  uint8_t TakeInt8(volatile uint8_t *data);
  bool CreateJSONPair(const char* type);
  void AddArray(void* src, uint32_t len);
  uint8_t* GetPointer(uint32_t index);
  void ToAsciiHex();
  void PackCC1101();
  bool UnpackCC1101();
  void SPrintf(const char* format, ...);
  void SPrintf(const char* format, __Va_list l);
  
  template <typename T> void Add(T value)
  {
    uint32_t max = sizeof(T) + length;
    if(max<bufferSize)
    {
      memcpy(&buffer[length],&value,sizeof(T));
      length += sizeof(T);
    }
  }
  template <typename T> T Get(uint32_t location)
  {
    static_assert(std::is_arithmetic<T>::value, "Arythmetic type required");
    T ans = 0;
    memcpy(&ans,&buffer[location],sizeof(T));
    return ans;
  }
  template <typename T> void Get(uint32_t location, T* ptr)
  {
    memcpy(ptr,buffer+location,sizeof(T));
  }
  
  int16_t GetId();
  void Clear();
  void AddCr();
  void AddCrLf();
  void CopyBuffer(BufferHandler* src, uint32_t start);
  uint32_t Idx();
  uint32_t Length();
  uint32_t Size();
  void ToXTEA();
  void FromXTEA();
  bool ToB64();
  bool FromB64();
  bool SimplePack(bool enc);
  bool SimpleUnpack();
  bool Discard(uint16_t l);
  bool Shift(uint16_t l);
  void PackForSL03x();
  void RotateLeft(uint16_t pos, uint16_t len);
  void Xor(uint16_t pos, uint8_t* op2, uint16_t len);
  bool UnpackFromSL03x();
  static void LoadKey(uint8_t* tmp);
protected:
  static void encBlock(uint32_t* data);
  static void decBlock(uint32_t* data);
  static uint8_t fromB64Char(uint8_t b64char);
  static uint8_t toAsciiChar(int8_t data);
  void pad(uint32_t padding);
  static const uint32_t xteaRounds = 32;
  uint8_t getChk();
  uint32_t bufferSize;
  uint32_t length;
  uint32_t idx;
  uint8_t* buffer;
  uint16_t id;
private:
};

#endif /* BUFFER_H_ */
