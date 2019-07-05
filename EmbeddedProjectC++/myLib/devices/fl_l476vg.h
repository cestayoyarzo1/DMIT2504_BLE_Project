/*
  FILE: fl_l476vg.h
  Created on: 15/5/2017, by Carlos Estay / Tom Diaz
*/

#ifndef FL_L476VG_H
#define FL_L476VG_H

//#include "x_nvmem.h"
//#include "stm32l4xx.h"

class FL_L476VG// : public x_NVMem
{
public:
  void Init(FLASH_TypeDef* flash_);
  void EraseSector(uint32_t pNumber);
  void WriteBurst(uint32_t address,void* src, uint32_t length);
  void ReadBurst(uint32_t addr,void* dst,uint32_t length);
//protected:
  void Unlock();
  void Lock();
  bool IsLocked();
  void ClearFlags();
//private:
  FLASH_TypeDef* flash;
  bool eraseError;
  bool writeError;
};

#endif /* FL_L476VG_H */
