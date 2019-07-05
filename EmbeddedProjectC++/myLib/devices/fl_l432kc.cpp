/*
  FILE: fl_l432kc.cpp
  Created on: 5/11/2018, by Tom Diaz
*/

#include "hal.h"
#include "fl_l432kc.h"
#include "string.h"

#define FLASH_KEY1      0x45670123
#define FLASH_KEY2      0xCDEF89AB

void FL_L432KC::Init(FLASH_TypeDef* flash_)
{
  flash = flash_;
  flash->CR |= FLASH_CR_EOPIE;          //Enable EOP int/flag
  //partNo = MemoryIC::FL_STM32L476VG;
}
//------------------------------------------------------------------------------
void FL_L432KC::Unlock()
{
  while(flash->SR & FLASH_SR_BSY);      //Wait until no operation is going
  if(IsLocked());
  {//unclock flash inserting keys
    flash->KEYR = FLASH_KEY1;          
    flash->KEYR = FLASH_KEY2;
  }
}
//------------------------------------------------------------------------------
void FL_L432KC::Lock()
{
  while(flash->SR & FLASH_SR_BSY);      //Wait until no operation is going
  FLASH->CR |= FLASH_CR_LOCK;          //Lock Flash
}
//------------------------------------------------------------------------------
bool FL_L432KC::IsLocked()
{
  if(flash->CR & FLASH_CR_LOCK)        //Check if flash is locked
    return true;
    else
      return false;
}
//------------------------------------------------------------------------------
void FL_L432KC::ClearFlags()
{
  
  flash->SR |= FLASH_SR_PGSERR;                 //Clear prog sequence error
  if(flash->SR & FLASH_SR_PROGERR)
    flash->SR |= FLASH_SR_PROGERR;              //Clear prog error
  if(flash->SR & FLASH_SR_SIZERR)
    flash->SR |= FLASH_SR_SIZERR;               //Clear Size error
  if (flash->SR & FLASH_SR_PGAERR)
    flash->SR |= FLASH_SR_PGAERR;               //Clear prog alignment error
  if(flash->SR & FLASH_SR_WRPERR);
    flash->SR |= FLASH_SR_WRPERR;                //Clear write protection error
  if(flash->SR & FLASH_SR_MISERR)
    flash->SR |= FLASH_SR_MISERR;               //Clear fast prog data miss error
  if(flash->SR & FLASH_SR_FASTERR);
    flash->SR |= FLASH_SR_FASTERR;              //Clear fast prog error
    
}
//------------------------------------------------------------------------------
void FL_L432KC::EraseSector(uint32_t sector)
{
  uint32_t pNumber = (sector & 0x000FFF00) / 0x800;
  if (IsLocked())
  Unlock();
  
  while(flash->SR & FLASH_SR_BSY);      //Wait until no operation is going 
  if (flash->SR & FLASH_SR_PGSERR)      //check for programming errors
    ClearFlags();
  flash->CR |= FLASH_CR_PER;            //Enable page erase
  flash->CR &= ~FLASH_CR_PNB;           //clear page number
  /*flash->CR &= ~FLASH_CR_BKER;          //Clear bank No  
  if(pNumber>255)
    flash->CR |= (pNumber-256)<<3 | FLASH_CR_BKER;  //page is  on bank 2
  else*/
  flash->CR |= pNumber<<3;                       //page in bank 1
  flash->CR |= FLASH_CR_STRT;           //Start erasing
  while(flash->SR & FLASH_SR_BSY);      //Wait until ready  
  if (flash->SR & FLASH_SR_EOP)         //Check that operation is completed and successful 
    flash->SR |= FLASH_SR_EOP;          //Clear flag, sucess  
  else
    eraseError=true;                    //Error handling to be added later eventually
  flash->CR &= ~FLASH_CR_PER;           //Disabe page erase

  Lock();
}
//------------------------------------------------------------------------------
void FL_L432KC::WriteBurst(uint32_t addr,void* src, uint32_t length)
{
  uint32_t data[2];
  uint8_t* source = (uint8_t*)src;
  
  if(length > 0)
  {
    if (IsLocked())
      Unlock();
    flash->CR |= FLASH_CR_PG;                     //Enable programming
  }

  while(length > 0)
  {
    data[0] = 0xffffffff;
    data[1] = 0xffffffff;
    
    if(length>7)
    {
      memcpy(&data[0],source,8);
      source += 8;
      length -= 8;
    }
    else
    {
      memcpy(&data[0],source,length);
      source += length;
      length -= length;
    }

    while(flash->SR & FLASH_SR_BSY);       //Wait until no operation is going  

    if (flash->SR & FLASH_SR_PGSERR)      //check for programming errors
    ClearFlags();

    *((__IO uint32_t*)(addr)) = data[0];
    *((__IO uint32_t*)(addr+4)) = data[1];
    addr += 8;
    
    while(flash->SR & FLASH_SR_BSY);       //Wait until no operation is going  
    //while(!(flash->SR & FLASH_SR_EOP))
    //flash->SR &= ~FLASH_SR_EOP;
  }
  
  
  /*while(flash->SR & FLASH_SR_BSY);            //Wait until ready
  if (flash->SR & FLASH_SR_EOP)               //Check that operation is completed and successful 
    flash->SR |= FLASH_SR_EOP;                //Clear flag, sucess
  else
    writeError=true;                          //error handling to be added later eventually
*/
  flash->CR &= ~FLASH_CR_PG;                    //Disable programming

  Lock();
}
//------------------------------------------------------------------------------
void FL_L432KC::ReadBurst(uint32_t addr,void* dst,uint32_t length)
{
  memcpy(dst,(void*)addr,length);
}