/*
* i2cport.cpp
*
*  Created on: Jun 20, 2016
*      Author: E1216431
*/

#include "hal.h"
#include "i2cport.h"
// NOTE:

//--------------------------------------------------------------------------------------------
void I2CPort::Init(I2C_TypeDef* i2c_)
{
  Buffer.Init(buff,I2C_BUFFER_SIZE);
  i2c = i2c_;
  timeout = 1;
  tout.Start();
}
//--------------------------------------------------------------------------------------------
// Set up I2C Buffer object before using this method
I2CResponse I2CPort::Write(uint16_t devAddress, uint8_t regAddress)
{
  uint16_t addr = (devAddress << 1) | 0x00; // Add R/W Bit
  uint16_t len = Buffer.Length();
  I2CResponse response = waitBusy();
  if(response == I2CResponse::Success)
  {
    if(len)
    {
      if(len > 1)
      {
        setupTransfer(addr,Buffer.Length(), TRANSMIT, I2C_RELOAD_MODE, false);
        response = memWrite(regAddress);
        if(response == I2CResponse::Success)
          response = transmit();
      }
      else
      {
        tout.Reset();
        setupTransfer(addr,1, TRANSMIT, I2C_CR2_AUTOEND, false);
        while(!(i2c->ISR & I2C_ISR_TXIS)) // Wait until it is safe to put data in Tx Register
        {
          if(tout.Read()>=timeout)
            return I2CResponse::Timeout;
        }
        i2c->TXDR = Buffer.Get<uint8_t>(0); 				// Send register address
        i2c->ICR |= I2C_ICR_STOPCF;	// Clear STOPF flag from ISR
      }
      Buffer.Clear();
    }
    else
      response = I2CResponse::Error;
    
    return response;
  }
  return I2CResponse::Busy;
}
//--------------------------------------------------------------------------------------------
I2CResponse I2CPort::WriteByte(uint16_t devAddress, uint8_t regAddress, uint8_t data)
{
  uint16_t addr = (devAddress << 1) | 0x00; // Add R/W Bit
  I2CResponse response = waitBusy();
  if(response == I2CResponse::Success)
  {
    setupTransfer(addr,1, TRANSMIT, I2C_RELOAD_MODE, true);
    response = memWrite(regAddress);
    if(response == I2CResponse::Success)
    {
      tout.Reset();
      setupTransfer(addr,1, TRANSMIT, I2C_CR2_AUTOEND, false);
      while(!(i2c->ISR & I2C_ISR_TXIS)) // Wait until it is safe to put data in Tx Register
      {
        if(tout.Read()>=timeout)
          return I2CResponse::Timeout;
      }
      i2c->TXDR = data; 				// Send register address
      i2c->ICR |= I2C_ICR_STOPCF;	// Clear STOPF flag from ISR
    }
    return response;
  }
  return I2CResponse::Busy;
}
//--------------------------------------------------------------------------------------------
I2CResponse I2CPort::Read(uint16_t devAddress, uint8_t regAddress, uint16_t length)
{
  uint16_t addr = (devAddress << 1) | 0x01; // Add R/W Bit
  I2CResponse response = waitBusy();
  if(response == I2CResponse::Success)
  {
    if(length <= I2C_BUFFER_SIZE)
    {
      setupTransfer(addr, 1, TRANSMIT, I2C_SOFTEND_MODE, true);
      response = writeByte(regAddress);
      
      if(response == I2CResponse::Success)
      {
        setupTransfer(addr, length, RECEIVE, I2C_AUTOEND_MODE, true);
        response = receive(length);
      }
      else
        response = I2CResponse::Error;
      return response;
    }
    return I2CResponse::Error;
  }
  return I2CResponse::Busy;
}
//--------------------------------------------------------------------------------------------
I2CResponse I2CPort::ReadByte(uint16_t devAddress, uint8_t regAddress)
{
  uint16_t addr = (devAddress << 1) | 0x01; // Add R/W Bit
  I2CResponse response = waitBusy();
  if(response == I2CResponse::Success)
  {
    /*if(length <= I2C_BUFFER_SIZE)
    {*/
      setupTransfer(addr, 1, TRANSMIT, I2C_SOFTEND_MODE, true);
      response = writeByte(regAddress);
      
      if(response == I2CResponse::Success)
      {
        setupTransfer(addr, 1, RECEIVE, I2C_AUTOEND_MODE, true);
        response = receive(1);
      }
      else
        response = I2CResponse::Error;
      return response;
    //}
    //return I2CResponse::Error;
  }
  return I2CResponse::Busy;
}
//--------------------------------------------------------------------------------------------
void I2CPort::setupTransfer(uint16_t devAddress, uint16_t nBytes,
                            bool read, uint32_t mode, bool start)
{  
  uint32_t tmpreg = i2c->CR2;
  tmpreg &= (uint32_t)~((uint32_t)(I2C_CR2_SADD | I2C_CR2_NBYTES | I2C_CR2_RELOAD | I2C_CR2_AUTOEND | I2C_CR2_RD_WRN | I2C_CR2_START | I2C_CR2_STOP));
  //TODO: Handle 8 and 10 bit addresses as well
  tmpreg |= devAddress  & I2C_CR2_SADD;      // Set 7 Bit address (SADD)
  tmpreg |= read?I2C_CR2_RD_WRN:0;	                        // Set Read
  tmpreg |= mode;	                                        // Set Auto STOP
  tmpreg |= nBytes<<16;				                // Set number of bytes to be sent (NBYTES)
  tmpreg |= start ? I2C_CR2_START : 0;		                        // Set START condition
  i2c->CR2 = tmpreg;
}
//--------------------------------------------------------------------------------------------
I2CResponse I2CPort::transmit()
{
  I2CResponse response = I2CResponse::Success;
  uint16_t sent = 0;
  while(sent<Buffer.Length())
  {
    //response = writeByte(buff[sent]);
    i2c->TXDR = Buffer.Get<uint8_t>(sent);
    if(response != I2CResponse::Success)
      return response;
    sent+=1;
  }
  tout.Reset();
  while(!(i2c->ISR & I2C_ISR_STOPF)) // Wait until STOP is complete
  {
    if(tout.Read()>=timeout)
      return I2CResponse::Timeout;
  }
  i2c->ICR |= I2C_ICR_STOPCF;	// Clear STOPF flag from ISR
  Buffer.Clear();
  return I2CResponse::Success;
}
//--------------------------------------------------------------------------------------------
I2CResponse I2CPort::writeByte(uint8_t data)
{
  tout.Reset();
  while(!(i2c->ISR & I2C_ISR_TXIS)) // Wait until it is safe to put data in Tx Register
  {
    if(tout.Read()>=timeout)
      return I2CResponse::Timeout;
  }
  i2c->TXDR = data; 				// Send register address
  while(!(i2c->ISR & I2C_ISR_TC));
  return I2CResponse::Success;
}
//--------------------------------------------------------------------------------------------
I2CResponse I2CPort::memWrite(uint8_t data)
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
I2CResponse I2CPort::receive(uint16_t nBytes)
{
  uint16_t size = nBytes;
  Buffer.Clear();
  tout.Reset();
  while(size>0)
  {
    while(!(i2c->ISR & I2C_ISR_RXNE)) // Wait until receiving byte
    {
      if(tout.Read()>=timeout)
        return I2CResponse::Timeout;
    }
    tmpByte = i2c->RXDR;
    Buffer.Add<uint8_t>(tmpByte);
    size-=1;
  }
  
  while(!(i2c->ISR & I2C_ISR_STOPF)); // Wait until STOP is complete
  i2c->ICR |= I2C_ICR_STOPCF;	// Clear STOPF flag from ISR
  return I2CResponse::Success;
}
//-------------------------------------------------------------------------------------------
// Check if bus is busy, if so, wait until timeout.
I2CResponse I2CPort::waitBusy()
{
  tout.Reset();
  while(i2c->ISR & I2C_ISR_BUSY)
  {
    if(tout.Read()>=timeout)
      return I2CResponse::Timeout;
  }
  return I2CResponse::Success;
}
