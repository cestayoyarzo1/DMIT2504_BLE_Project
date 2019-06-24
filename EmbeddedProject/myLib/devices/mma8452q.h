/*
 * mma8452q.h
 *
 *  Created on: Jun 24, 2016
 *      Author: E1216431
 */

#ifndef MMA8452Q_H_
#define MMA8452Q_H_

#include "x_i2cport.h"

class MMA8452Q// : public Accelerometer
{
public:
  void Init(x_I2CPort* i2c_, uint16_t devAddress);
  void Read();
  int16_t GetSample(uint8_t sample);
private:
  x_I2CPort* i2c;
  uint32_t count;
  uint8_t address;
  int16_t samples[3];
  uint8_t buffer[8];
};

#endif /* MMA8452Q_H_ */
