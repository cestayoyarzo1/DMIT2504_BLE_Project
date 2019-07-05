/*
  FILE: iputils.cpp
  Created on: 8/28/2017, by Tom Diaz
*/

#include <iputils.h>

//******************************************
//******************************************
//********** SWAP BYTES IN A WORD **********
//******************************************
//******************************************
uint16_t IPUtils::swap_word_bytes(uint16_t data)
{
  WORD_VAL new_data;
  uint8_t b_temp;
  
  new_data.Val = data;
  b_temp = new_data.v[1];
  new_data.v[1] = new_data.v[0];
  new_data.v[0] = b_temp;
  
  return (new_data.Val);
}
//************************************************
//************************************************
//********** CONVERT BYTE TO LOWER CASE **********
//************************************************
//************************************************
uint8_t IPUtils::convert_character_to_lower_case (uint8_t character)
{
  if ((character >= 'A') && (character <= 'Z'))
    return(character + 0x20);
  else
    return(character);
}	

//*******************************************
//*******************************************
//********** SWAP BYTES IN A DWORD **********
//*******************************************
//*******************************************
uint32_t IPUtils::swap_dword_bytes(uint32_t data)
{
  DWORD_VAL old_data;
  DWORD_VAL new_data;
  
  old_data.Val = data;
  
  new_data.v[0] = old_data.v[3];
  new_data.v[1] = old_data.v[2];
  new_data.v[2] = old_data.v[1];
  new_data.v[3] = old_data.v[0];
  
  return (new_data.Val);
}
