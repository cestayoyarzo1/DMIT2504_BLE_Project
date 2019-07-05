/*
  FILE: ee_l0xx.cpp
  Created on: 12/5/2017, by Tom Diaz
*/

#include <hal.h>
#include <ee_l0xx.h>

void EE_L0xx::Init(FLASH_TypeDef* flash_, uint32_t* start_, uint32_t len_)
{
  start = start_;
  len = len;
  flash = flash_;
}
//------------------------------------------------------------------------------
bool EE_L0xx::Write(void *data, uint32_t address, uint32_t len)
{
  uint8_t* ptr = (uint8_t*)data;
  
  if((address & 0x03) == 0) // Aligns to 32 bit
  {
    /* (1) Wait till no operation is on going */
    /* (2) Check if the PELOCK is unlocked */
    /* (3) Perform unlock sequence */
    while ((flash->SR & FLASH_SR_BSY) != 0) /* (1) */
    {
      /* For robust implementation, add here time-out management */
    }
    if ((FLASH->PECR & FLASH_PECR_PELOCK) != 0) /* (2) */
    {
      flash->PEKEYR = FLASH_PEKEY1; /* (3) */
      flash->PEKEYR = FLASH_PEKEY2;
    }
    
    uint32_t dataHolder = 0;
    
    while(len > 0)
    {
      if(len >= sizeof(uint32_t))
      {
        memcpy(&dataHolder, ptr, sizeof(uint32_t));
        *(start + address) = dataHolder;
        len -= 4;
        address += 1; // uint32_t pointer
        ptr += 4;
      }
      else
      {
        memcpy(&dataHolder, ptr, len);
        *(start + address) = dataHolder;
      }
    }    
    
    /* (1) Wait till no operation is on going */
    /* (2) Locks the NVM by setting PELOCK in PECR */
    while ((flash->SR & FLASH_SR_BSY) != 0) /* (1) */
    {
      /* For robust implementation, add here time-out management */
    }
    flash->PECR |= FLASH_PECR_PELOCK; /* (2) */
  }
  return false;
}
//------------------------------------------------------------------------------
bool EE_L0xx::Read(void *data, uint32_t address, uint32_t len)
{
  memcpy(data, start+address, len);
  return true;
}