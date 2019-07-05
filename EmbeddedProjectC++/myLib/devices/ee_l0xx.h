/*
  FILE: ee_l0xx.h
  Created on: 12/5/2017, by Tom Diaz
*/
#ifndef EEPROML0XX_H
#define EEPROML0XX_H

class EE_L0xx
{
public:
  EE_L0xx(){};
  ~EE_L0xx(){};
  void Init(FLASH_TypeDef* flash_, uint32_t* start_, uint32_t len_);
  bool Write(void* data, uint32_t address, uint32_t len);
  bool Read(void* data, uint32_t address, uint32_t len);
protected:
  FLASH_TypeDef* flash;
  uint32_t len;
  uint32_t* start;
private:
};


#endif /* EEPROML0XX_H */
