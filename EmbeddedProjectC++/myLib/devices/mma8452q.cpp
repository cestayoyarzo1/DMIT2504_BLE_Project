/*
* mma8452q.cpp
*
*  Created on: Jun 24, 2016
*      Author: E1216431
*/

#include "hal.h"
#include "mma8452q.h"
#define CTRL_REG1 0x2A
#define STATUS 0x00
#define OUT_X_MSB 0x01

void MMA8452Q::Init(x_I2CPort* i2c_, uint16_t devAddress)
{
  i2c = i2c_;
  address = devAddress;
  // Low Noise, 50Hz data rate, Wake-up. STANDBY -> ACTIVE
  i2c->WriteByte(address,CTRL_REG1,0x25);
  // Read id
  buffer[7] = i2c->ReadByte(address, 0x0d);
}
//------------------------------------------------------------------------------------
void MMA8452Q::Read()
{
  i2c->Read(address, STATUS, buffer, 7);
  if (i2c->Response() == I2CResponse::Success)
  {    
    /*samples[0] = ((buffer[1] << 8) | buffer[2]);
    samples[1] = ((buffer[3] << 8) | buffer[4]);
    samples[2] = ((buffer[5] << 8) | buffer[6]);*/
    
    samples[0] = buffer[1];
    samples[1] = buffer[3];
    samples[2] = buffer[5];
  }
}
//-----------------------------------------------------------------------------------
int16_t MMA8452Q::GetSample(uint8_t sample)
{
  if(sample<3)
    return samples[sample];
  else
    return -1;
}

