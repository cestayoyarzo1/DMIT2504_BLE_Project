/*
 * spiport.h
 *
 *  Created on: Sep 16, 2016
 *      Author: E1216431
 */

#ifndef SPIPORT_H_
#define SPIPORT_H_

#include <x_spiport.h>

class SPIPort : public x_SPIPort
{
public:
  void Init(SPI_TypeDef* spi_);
  void WriteByte(uint8_t byte);
  uint8_t ReadByte();
  void SetClock(uint8_t clk);
  void Wait();
private:
  SPI_TypeDef* spi;
  uint8_t* dr;
};

#endif /* SPIPORT_H_ */
