/*
  FILE: stm32i2cport.cpp
  Created on: 7/12/2018, by Tom Diaz
*/

#include <hal.h>
#include "stm32i2cport.h"

#define GENERIC_TIMEOUT 1

void Stm32I2CPort::Init(I2C_TypeDef* i2c_)
{
  i2c = i2c_;
  timeout = 1;
  tout.Start();
}
//------------------------------------------------------------------------------
void Stm32I2CPort::Read(uint16_t devAddress, uint8_t regAddress, uint8_t* buff, uint16_t length)
{
  uint16_t addr = (devAddress << 1) | 0x01; // Add R/W Bit
  response = waitBusy();
  if(response == I2CResponse::Success)
  {
      setupTransfer(addr, 1, TRANSMIT, I2C_SOFTEND_MODE, true);
      response = requestMemRead(regAddress);
      
      if(response == I2CResponse::Success)
      {
        setupTransfer(addr, length, RECEIVE, I2C_AUTOEND_MODE, true);
        response = receive(buff, length);
      }
  }
}
//------------------------------------------------------------------------------
void Stm32I2CPort::Write(uint16_t devAddress, uint8_t regAddress, uint8_t* buff, uint16_t length)
{
  uint16_t addr = (devAddress << 1) | 0x00; // Add R/W Bit
  response = waitBusy();
  uint16_t sent = 0;
  if(response == I2CResponse::Success)
  {
    if(length)
    {
      setupTransfer(addr,1, TRANSMIT, I2C_RELOAD_MODE, true);
      response = requestMemWrite(regAddress);
      if(response == I2CResponse::Success)
      {
        tout.Reset();
        setupTransfer(addr, length, TRANSMIT, I2C_CR2_AUTOEND, false);
        while(!(i2c->ISR & I2C_ISR_TXIS)) // Wait until it is safe to put data in Tx Register
        {
          if(tout.Read()>=timeout)
          {
            response = I2CResponse::Timeout;
            return;
          }
        }
        while(sent < length)
        {
          i2c->TXDR = buff[sent++]; 				// Send data
        }
        i2c->ICR |= I2C_ICR_STOPCF;	// Clear STOPF flag from ISR
 
      }
    }
    else
      response = I2CResponse::Error;
  }
  else
    response = I2CResponse::Busy;
}
//--------------------------------------------------------------------------------------------
void Stm32I2CPort::setupTransfer(uint16_t devAddress, uint16_t nBytes,
                            bool read, uint32_t mode, bool start)
{  
  uint32_t tmpreg = i2c->CR2;
  tmpreg &= (uint32_t)~((uint32_t)(I2C_CR2_SADD | I2C_CR2_NBYTES | I2C_CR2_RELOAD
                                   | I2C_CR2_AUTOEND | I2C_CR2_RD_WRN 
                                     | I2C_CR2_START | I2C_CR2_STOP));
  //TODO: Handle 8 and 10 bit addresses as well
  tmpreg |= devAddress  & I2C_CR2_SADD;         // Set 7 Bit address (SADD)
  tmpreg |= read?I2C_CR2_RD_WRN:0;	        // Set Read
  tmpreg |= mode;	                        // Set Auto STOP
  tmpreg |= nBytes<<16;		                // Set number of bytes to be sent (NBYTES)
  tmpreg |= start ? I2C_CR2_START : 0;		// Set START condition
  i2c->CR2 = tmpreg;
}
//-------------------------------------------------------------------------------------------
// Check if bus is busy, if so, wait until timeout.
I2CResponse Stm32I2CPort::waitBusy()
{
  tout.Reset();
  while(i2c->ISR & I2C_ISR_BUSY)
  {
    if(tout.Read()>=timeout)
      return I2CResponse::Timeout;
  }
  return I2CResponse::Success;
}
//--------------------------------------------------------------------------------------------
I2CResponse Stm32I2CPort::requestMemRead(uint8_t data)
{
  if(waitOnStatusFlag(I2C_ISR_TXIS, GENERIC_TIMEOUT) == I2CResponse::Success)
  {
    i2c->TXDR = data; 				// Send register address
    return waitOnStatusFlag(I2C_ISR_TC, GENERIC_TIMEOUT);
  }
  return I2CResponse::Timeout;
}
//--------------------------------------------------------------------------------------------
I2CResponse Stm32I2CPort::requestMemWrite(uint8_t data)
{
  tout.Reset();
  while(!(i2c->ISR & I2C_ISR_TXIS)) // Wait until it is safe to put data in Tx Register
  {
    if(tout.Read()>=timeout)
      return I2CResponse::Timeout;
  }
  i2c->TXDR = data; 				// Send register address
  while(!(i2c->ISR & I2C_ISR_TCR));
  return I2CResponse::Success;
}
//--------------------------------------------------------------------------------------------
I2CResponse Stm32I2CPort::receive(uint8_t* buff, uint16_t nBytes)
{
  uint16_t size = nBytes;
  uint16_t i = 0;
  tout.Reset();
  while(size>0)
  {
    /*while(!(i2c->ISR & I2C_ISR_RXNE)) // Wait until receiving byte
    {
      if(tout.Read()>=timeout)
        return I2CResponse::Timeout;
  }*/
    if(waitOnStatusFlag(I2C_ISR_RXNE, GENERIC_TIMEOUT) == I2CResponse::Success)
    {
      buff[i++] = i2c->RXDR;
      size-=1;
    }
    else
      return I2CResponse::Timeout;
  }
  
  while(!(i2c->ISR & I2C_ISR_STOPF)); // Wait until STOP is complete
  i2c->ICR |= I2C_ICR_STOPCF;	// Clear STOPF flag from ISR
  return I2CResponse::Success;
}
//------------------------------------------------------------------------------
// Generic Interrupt Status Register flag checking routine with integrated timeout.
I2CResponse Stm32I2CPort::waitOnStatusFlag(uint32_t flag, uint32_t t)
{
  Stopwatch timer;
  timer.Start();
  while(!(i2c->ISR & flag)) // Wait until receiving byte
  {
    if(tout.Read() >= t)
      return I2CResponse::Timeout;
  }
  return I2CResponse::Success;
}

