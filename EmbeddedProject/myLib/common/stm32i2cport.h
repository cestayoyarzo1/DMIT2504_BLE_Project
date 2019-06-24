/*
  FILE: stm32i2cport.h
  Created on: 7/12/2018, by Tom Diaz
*/
#ifndef STM32I2CPORT_H
#define STM32I2CPORT_H

#include "x_i2cport.h"

class Stm32I2CPort : public x_I2CPort
{
public:
  void Init(I2C_TypeDef* i2c_);
  void Read(uint16_t devAddress, uint8_t regAddress, uint8_t* buff, uint16_t length);
  void Write(uint16_t devAddress, uint8_t regAddress, uint8_t* buff, uint16_t length);
protected:
  I2C_TypeDef* i2c;
  uint16_t timeout;
  I2CResponse waitBusy();
  void setupTransfer(uint16_t devAddress, uint16_t nBytes, bool read, uint32_t mode, bool start);
  I2CResponse requestMemRead(uint8_t data);
  I2CResponse requestMemWrite(uint8_t data);
  I2CResponse receive(uint8_t* buff, uint16_t nBytes);
  I2CResponse waitOnStatusFlag(uint32_t flag, uint32_t timeout);
  //I2CResponse transmit(uint8_t* buff, uint16_t nBytes);
private:
};

#endif /* STM32I2CPORT_H */
