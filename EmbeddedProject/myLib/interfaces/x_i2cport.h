#ifndef X_I2CPORT_H_
#define X_I2CPORT_H_

#include "stopwatch.h"
#include "buffer.h"

enum class I2CResponse
{
  NoResponse,
  Busy,
  Timeout,
  Error,
  Success,
};

class x_I2CPort
{
public:
  virtual void Read(uint16_t devAddress, uint8_t regAddress, uint8_t* buff, uint16_t length) = 0;
  virtual void Write(uint16_t devAddress, uint8_t regAddress, uint8_t* buff, uint16_t length) = 0;
  uint8_t ReadByte(uint16_t devAddress, uint8_t regAddress);
  void WriteByte(uint16_t devAddress, uint8_t regAddress, uint8_t data);
  I2CResponse Response() { return response; } 
protected:
  I2CResponse response;
  uint8_t tmpByte;
  uint16_t timeout;
  Stopwatch tout;
};

#endif /* X_I2CPORT_H_ */
