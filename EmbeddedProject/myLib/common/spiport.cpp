/*
 * spiport.cpp
 *
 *  Created on: June 24th, 2019
 *      Author: Carlos Estay
 */

#include <hal.h>
#include <spiport.h>

//------------------------------------------------------------------------------
void SPIPort::Init(SPI_TypeDef* spi_)
{
  spi = spi_;
  dr = (uint8_t*)&spi->DR;
  SetClock(SpiClk::fclk_16);
}
//------------------------------------------------------------------------------
void SPIPort::WriteByte(uint8_t byte)
{
  *dr = byte;
  while(!(spi->SR & SPI_SR_RXNE));
  byte = *dr;
}
//------------------------------------------------------------------------------
uint8_t SPIPort::ReadByte()
{
  *dr = 0;
  while(!(spi->SR & SPI_SR_RXNE)); //Wait until RX register is not empty
  tmpByte = *dr; //Read Rx register
  
  return tmpByte;
}
//------------------------------------------------------------------------------
void SPIPort::SetClock(uint8_t clk)
{
  clock = clk;
  spi->CR1 &= ~SPI_CR1_SPE; // Disable SPI
  spi->CR1 |= SPI_CR1_SSM; // Enable Software NSS Management
  spi->CR1 |= SPI_CR1_SSI; // Set Soft NSS Hi
  spi->CR1 |= SPI_CR1_MSTR; // Set Master mode
  spi->CR1 &= ~(7<<3);        //Clear clock
  spi->CR1 |= clk<<3;         //Set clock
  spi->CR1 |= SPI_CR1_SPE; // Re-Enable SPI2
}
//------------------------------------------------------------------------------
inline void SPIPort::Wait()
{
  while(!(spi->SR & SPI_SR_RXNE));
}

