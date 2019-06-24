/*
  FILE: flash.h
  Created on: 6/29/2016, by Tom Diaz
*/

#ifndef FLASH_MEM_H
#define FLASH_MEM_H

#include "stdint.h"
#include "global_constants.h"
#include "x_spiport.h"

// 4Mb Flash 1024 (0x400) sectors
// 4096 (0x1000) bytes per sector

//Flash map
//0x0000 Parameters
//0x8000 Labels
//0x9000 CycleCount
//0xA000 SD Condition

#include "x_nvmem.h"
#include "x_gpio.h"
  
class SPIFlash : public x_NVMem
{
public:
  SPIFlash(){};
  ~SPIFlash(){};
  void Init(x_SPIPort* spi_, x_GPIO* cs_);
  void ReadBurst(uint32_t address,void* dst,uint32_t length);
  void WriteBurst(uint32_t address,void* src,uint32_t length);
  void EraseSector(uint32_t sector);
protected:
  bool isBusy();
  void WriteByte(uint32_t address, uint8_t data);
  uint8_t ReadByte(uint32_t address);
  virtual void setAddress(uint32_t address);
  void getID();
  x_SPIPort* spi;
  x_GPIO* cs;
  uint8_t readStatus();
private:
  void writeEnable(bool state);
};


class SPIFram : public SPIFlash
{
public:
  SPIFram(){};
  ~SPIFram(){};
protected:
  void setAddress(uint32_t address);
private:
};

#endif /* FLASH_MEM_H */
