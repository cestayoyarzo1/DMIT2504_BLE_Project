/*
  FILE: fl_l432kc.h
  Created on: 5/11/2018, by Tom Diaz
*/
#ifndef FL_L432KC_H
#define FL_L432KC_H

class FL_L432KC// : public x_NVMem
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

#endif /* FL_L432KC_H */
