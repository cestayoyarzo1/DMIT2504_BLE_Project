/*
 * x_spiport.h
 *
 *  Created on: June 24th, 2019
 *      Author: Carlos Estay
 */

   
   
  
#ifndef X_SPIPORT_H_
#define X_SPIPORT_H_

#include <stdint.h>

class x_SPIPort
{
public:
  x_SPIPort(){ }
  virtual void WriteByte(uint8_t byte) = 0;
  virtual uint8_t ReadByte() = 0;
  virtual void Wait() = 0;
  virtual void SetClock(uint8_t clk) = 0;
protected:
  uint32_t clock;
  uint8_t tmpByte;
};

#endif /* X_SPIPORT_H_ */
