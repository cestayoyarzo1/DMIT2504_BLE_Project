/*
  FILE: flash.cpp
  Created on: 6/29/2016, by Tom Diaz
*/

#include "nvmem.h"

#define BUSY_BIT 0x01
//#define NUMBER_OF_SECTORS               0x400000 //4Mb Flash
#define NUMBER_OF_SECTORS               2000 //8Mb Flash

#define WRITE_STATUS_REGISTER           0x01
#define READ_CONFIGURATION_REGISTER     0x35
#define WRITE_DISABLE                   0x04
#define STATUS_REG                      0x05
#define WRITE_ENABLE                    0x06
#define ERASE_4K_BLOCK                  0x20
#define READ_BYTE                       0x03
#define WRITE_BYTE                      0x02
#define WRITE_BLOCK_PROTECTION_REGISTER 0x42
#define WRITE_AUTO_ADDRESS_INCREMENT    0xAD
#define GLOBAL_LOCK_PROTECTION_UNLOCK   0x98

#define TEST_FLASH_BYTE                 0x1000
#define TEST_SECTOR TEST_FLASH_BYTE     0x1000

//------------------------------------------------------------------------------
//------------------------------SPIFlash----------------------------------------
//------------------------------------------------------------------------------

void SPIFlash::Init(x_SPIPort* spi_, x_GPIO* cs_)
{
  spi = spi_;
  cs = cs_;
  
  spi->SetClock(SpiClk::fclk_16);
  
  
  getID();
  
  //Disable write protection
  switch(partNo)
  {
  case MemoryIC::FL_SST26VF064B:
  case MemoryIC::FL_SST26VF032B:
    writeEnable(true);
    cs->Clear();
    spi->WriteByte(WRITE_STATUS_REGISTER);
    spi->WriteByte(0x02);
    spi->WriteByte(0x02);
    cs->Set();
    
    while(isBusy());
    
    writeEnable(true);
    cs->Clear();
    spi->WriteByte(GLOBAL_LOCK_PROTECTION_UNLOCK);
    cs->Set();
    break;
  case MemoryIC::FL_SST25VF032B:
  case MemoryIC::FL_W25Q32FV:
  case MemoryIC::FR_MB85RS64V:
  default:
    writeEnable(true);
    cs->Clear();
    spi->WriteByte(WRITE_STATUS_REGISTER);
    spi->WriteByte(0x00);
    cs->Set();
    break;
  }
}
//------------------------------------------------------------------------------
void SPIFlash::writeEnable(bool state)
{
  cs->Clear();
  spi->WriteByte(state?WRITE_ENABLE:WRITE_DISABLE);
  cs->Set();
}
//------------------------------------------------------------------------------
void SPIFlash::getID()
{
  uint32_t id = 0;
  //for(;;)
  //{
  cs->Clear();             //Pull CS Low
  
  spi->WriteByte(0x9F);         //Send Get JEDEC ID command
  id=0;
  id += spi->ReadByte();         //Read 1st id byte
  id<<=8;
  id += spi->ReadByte();         //Read 2nd id byte
  id<<=8;
  id += spi->ReadByte();         //Read 3rd id byte
  
  cs->Set();              //Pull CS High
  //}
  partNo = MemoryIC::Validate(id);
  //if(partNo==MemoryIC::Unknown) HAL::SetError(SysError::FlashId); //TODO
}
//------------------------------------------------------------------------------
bool SPIFlash::isBusy()
{
  return(readStatus()&0x01); //Return busy bit state
}
//------------------------------------------------------------------------------
uint8_t SPIFlash::readStatus()
{
  uint8_t st = 0;
  cs->Clear();
  spi->WriteByte(STATUS_REG);    
  st = spi->ReadByte();
  cs->Set();
  return st;
}
//------------------------------------------------------------------------------
void SPIFlash::ReadBurst(uint32_t address,void* dst,uint32_t length)
{
  uint8_t* ptr = (uint8_t*)dst;
  
  cs->Clear();
  
  spi->WriteByte(READ_BYTE);
  setAddress(address);
  for(uint32_t i=0; i<length; i++)
    ptr[i] = spi->ReadByte();
      
  cs->Set();
}
//------------------------------------------------------------------------------
void SPIFlash::WriteBurst(uint32_t address,void* src,uint32_t length)
{
  uint8_t* ptr = (uint8_t*)src;
  for(uint32_t i=0; i<length;i++)
    WriteByte(address+i, ptr[i]);
}
//------------------------------------------------------------------------------
void SPIFlash::EraseSector(uint32_t sector)
{
  /*if(sector<NUMBER_OF_SECTORS)
  {*/
    writeEnable(true);
    cs->Clear();
    spi->WriteByte(ERASE_4K_BLOCK);
    setAddress(sector);
    cs->Set();
    while(isBusy());
  //}
}
//------------------------------------------------------------------------------
void SPIFlash::WriteByte(uint32_t address, uint8_t data)
{
  writeEnable(true);
  cs->Clear();
  spi->WriteByte(WRITE_BYTE); //Command to write flash
  setAddress(address);
  spi->WriteByte(data);
  cs->Set();
  while(isBusy());
}
//------------------------------------------------------------------------------
uint8_t SPIFlash::ReadByte(uint32_t address)
{
  uint8_t ans = 0x00;
  cs->Clear();
  spi->WriteByte(READ_BYTE); //Command to read flash
  setAddress(address);
  ans = spi->ReadByte();
  cs->Set();
  return ans;
}
//------------------------------------------------------------------------------
void SPIFlash::setAddress(uint32_t address)
{
  spi->WriteByte((address>>16)&0xff);
  spi->WriteByte((address>>8)&0xff);
  spi->WriteByte(address&0xff);
}
//------------------------------------------------------------------------------
//--------------------------SPIFram---------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void SPIFram::setAddress(uint32_t address)
{
  spi->WriteByte((address>>8)&0xff);
  spi->WriteByte(address&0xff);
}
