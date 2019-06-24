/*
* i2cport.h
*
*  Created on: Jun 20, 2016
*      Author: E1216431
*/

#ifndef I2CPORT_H_
#define I2CPORT_H_

#include "x_i2cport.h"
#define I2C_BUFFER_SIZE 32

#define I2C_RELOAD_MODE I2C_CR2_RELOAD
#define I2C_AUTOEND_MODE I2C_CR2_AUTOEND
#define I2C_SOFTEND_MODE 0
#define TRANSMIT false
#define RECEIVE true

class I2CPort //: public x_I2CPort
{
public:
  void Init(I2C_TypeDef* i2c_);
  I2CResponse Read(uint16_t devAddress, uint8_t regAddress, uint16_t length);
  I2CResponse ReadByte(uint16_t devAddress, uint8_t regAddress);
  I2CResponse Write(uint16_t devAddress, uint8_t regAddress);
  I2CResponse WriteByte(uint16_t devAddress, uint8_t regAddress, uint8_t data);
  //Buffer TxRxBuffer;
  BufferHandler Buffer;
private:
  I2C_TypeDef* i2c;
  uint8_t buff[I2C_BUFFER_SIZE];
  uint16_t timeout;
  Stopwatch tout;
  uint8_t tmpByte;
  void setupTransfer(uint16_t devAddress, uint16_t nBytes, bool read, uint32_t mode, bool start);
  I2CResponse receive(uint16_t nBytes);
  I2CResponse transmit();
  I2CResponse writeByte(uint8_t data);
  I2CResponse memWrite(uint8_t data);
  I2CResponse waitBusy();
};

#endif /* I2CPORT_H_ */
