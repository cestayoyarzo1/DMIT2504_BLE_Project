/*
  FILE: nmeadecoder.cpp
  Created on: 9/27/2017, by Tom Diaz
*/

#include <nmeadecoder.h>
#include <string.h>
#include <stdlib.h>
#include <hal.h>

#define NMEA_ID_LENGTH 6

bool NMEADecoder::Decode(BufferHandler* b)
{
  int32_t limit = b->Find('\n');
  if(limit > 0)
  {
    limit = (limit>MAX_NMEA_STING_LENGTH)?MAX_NMEA_STING_LENGTH:limit;
    b->GetArray(buffer,0,limit);
    b->Discard(limit+1);
    buffer[limit] = 0;
    if(!strncmp((char const*)buffer,"$GNGGA",NMEA_ID_LENGTH))
    {
      //HAL::Debug("%s\r\n",buffer);
      lock = GNGGA();
    }
    else
      buffer[0] = 0;
    return true;
  }
  else
  {
    buffer[0] = 0;
    return false;
  }
}
//------------------------------------------------------------------------------
bool NMEADecoder::GNGGA()
{
  int8_t commas[8];
  uint8_t scratch[16];
  int8_t j = 0;
  int8_t i = 0;
  while(buffer[i] && (j < 8))
  {
    if(buffer[i] == ',')
      commas[j++] = i;
    i++;
  }
  
  if(buffer[commas[2]+1]!=',')
    commas[6] = buffer[commas[2]+1];
  else
    return false;
  
  if(buffer[commas[4]+1]!=',')
    commas[7] = buffer[commas[4]+1];
  else
    return false;
  
  i = commas[2]-commas[1]-1;
  if(i>0)
  {
    memcpy(scratch,&buffer[commas[1]+1],i);
    scratch[i] = 0;
    getCoordenate(scratch,commas[6],&lat);
  }
  else
    return false;
  
  i = commas[4]-commas[3]-1;
  if(i>0)
  {
    memcpy(scratch,&buffer[commas[3]+1],i);
    scratch[i] = 0;
    getCoordenate(scratch,commas[7],&lon);
  }
  else
    return false;
  return true;
}
//------------------------------------------------------------------------------
bool NMEADecoder::getCoordenate(uint8_t* b, uint8_t sign, double* coord)
{
  int8_t i = 0;
  int8_t dot = -1;
  uint8_t* b2;
  uint8_t len = strlen((char const*)b);
  double a1=0,a2=0;
  
  while(b[i])
  {
    if(b[i] == '.')
    {
      dot = i;
      break;
    }
    i++;
  }
  
  if(dot > 0)
  {
    memmove(&b[dot-1],&b[dot-2],len-dot+2);
    b[dot-2] = 0;
    b2 = &b[dot-1];
    b2[len-dot+3] = 0;
    
    a1 = strtod((char const*)b,0);
    a2 = strtod((char const*)b2,0);
    
    a2 = a2 / 60.0f;
    *coord = a1 + a2;
    
    if((sign == 'S') || (sign == 'W'))
      *coord *= -1;
  }
  
  return true;
}
//------------------------------------------------------------------------------
uint8_t* NMEADecoder::GetBuffer()
{
  return buffer;
}