/*
  FILE: x_nvmem.h
  Created on: 15/5/2017, by Tom Diaz
*/

#ifndef X_NVMEM_H
#define X_NVMEM_H

#include "stdint.h"
#include "type_traits"

/*template< class T >
inline constexpr bool is_integral_v = std::is_integral<T>::value;*/

//----------------------------------------------------------------------MemoryIC
class MemoryIC
{
public:
  enum Value
  {
    Undefined=0,
    FL_SST25VF032B=0xBF254A,
    FL_SST26VF032B=0xBF2642,
    FL_SST26VF064B=0xBF2643,
    FL_W25Q32FV=0xEF4016,
    FR_MB85RS64V=0x047F03,
    FL_STM32L476VG=0xFFFFFE,
    Unknown=0xFFFFFF
  };
  static Value Validate(uint32_t val)
  {
    switch(val)
    {
    case FL_SST25VF032B: 
      return FL_SST25VF032B;
    case FL_SST26VF032B:
      return FL_SST26VF032B;
    case FL_SST26VF064B:
      return FL_SST26VF064B;
     case FL_W25Q32FV:
      return FL_W25Q32FV;
     case FR_MB85RS64V:
      return FR_MB85RS64V;
    default:
      return Unknown;
    }
  }
};

class x_NVMem
{
public:
  virtual void ReadBurst(uint32_t address,void* dst,uint32_t length) = 0;
  virtual void WriteBurst(uint32_t address,void* src,uint32_t length) = 0;
  virtual void EraseSector(uint32_t sector) = 0;
  template <typename T> T Read(uint32_t address)
  {
    static_assert(std::is_arithmetic<T>::value, "Arythmetic type required");
    T ans;
    ReadBurst(address, &ans, sizeof(T));
    return ans;
  }
  template <typename T> void Write(uint32_t address, T value)
  {
    static_assert(std::is_arithmetic<T>::value, "Arythmetic type required");
    WriteBurst(address, &value, sizeof(T));
  }
protected:
  MemoryIC::Value partNo;
};

#endif /* X_NVMEM_H */
