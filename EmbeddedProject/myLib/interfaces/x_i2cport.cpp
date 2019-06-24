
#include "x_i2cport.h"

uint8_t x_I2CPort::ReadByte(uint16_t devAddress, uint8_t regAddress)
{
  Read(devAddress, regAddress, &tmpByte, 1); 
  return tmpByte;
}

void x_I2CPort::WriteByte(uint16_t devAddress, uint8_t regAddress, uint8_t data)
{
  uint8_t byte = data;
  Write(devAddress, regAddress, &byte, 1);
}