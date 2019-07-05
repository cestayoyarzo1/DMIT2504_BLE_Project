/*
  FILE: nfccard.cpp
  Created on: 9/18/2016, by Tom Diaz
*/

#include <hal.h>
#include <nfccard.h>
#include <string.h>

//------------------------------------------------------------------------------
bool NfcCard::operator~()
{
  if(uidLength)
  {
    memset(this,0,sizeof(NfcCard));
    return true;
  }
  return false;
}
//------------------------------------------------------------------------------
bool NfcCard::operator==(NfcCard* card)
{
  if(uidLength==card->uidLength)
    return memcmp(uid,card->uid,uidLength);
  else
    return false;
}
//------------------------------------------------------------------------------
bool NfcCard::operator==(NfcCard card)
{
  if(uidLength==card.uidLength)
    return memcmp(uid,card.uid,uidLength);
  else
    return false;
}
//------------------------------------------------------------------------------
bool NfcCard::operator==(const char* uid_)
{
  for(uint8_t i=0;i<uidLength;i++)
  {
    if(uid[i] != uid_[i])
      return false;
  }
  return true;
}
//------------------------------------------------------------------------------
NfcCard& NfcCard::operator=(NfcCard* src)
{
  if(src->uidLength)
    memcpy(this,src,sizeof(NfcCard));
  return *this;
}
//------------------------------------------------------------------------------
NfcCard& NfcCard::operator=(NfcCard src)
{
  if(src.uidLength)
    memcpy(this,&src,sizeof(NfcCard));
  return *this;
}
//------------------------------------------------------------------------------
bool NfcCard::Init(unsigned char* buffer)
{
  bool different = false;
  unsigned int len = buffer[1]-4;
  different = memcmp(uid,buffer+4,len);
  if(different)
  {
    len = buffer[1]-4;
    memcpy(uid,buffer+4,len);
    uidLength=len;
    type = (CardType::Value)buffer[buffer[1]];
    extraDataIndex=0;
  }
  return different;
}
//------------------------------------------------------------------------------
void NfcCard::Serialize(BufferHandler* buff)
{
  buff->AddByte(type);
  if(uidLength) //If there is a uidLength the card is not empty
  {
    buff->AddByte(numberOfApplications);
    buff->AddByte(uidLength);
    buff->AddArray(uid,uidLength); // Add uid
    /*buff->AddInt16(hwVer);
    buff->AddInt16(swVer);
    buff->AddInt16(year);
    buff->AddInt16(week);
    buff->AddInt16(size);*/
  }
}
//------------------------------------------------------------------------------
bool NfcCard::Validate(unsigned char* k)
{
  for(unsigned int i=0;i<uidLength;i++)
  {
    if(uid[i]!=k[i]) return false;
  }
  return true;
}
//------------------------------------------------------------------------------
CardType::Value NfcCard::GetType()
{
  return type;
}
//------------------------------------------------------------------------------
uint8_t* NfcCard::GetUID()
{
  return uid;
}
//------------------------------------------------------------------------------
uint8_t NfcCard::GetUIDLength()
{
  return uidLength;
}
//------------------------------------------------------------------------------
void NfcCard::AddExtraData(unsigned char* data)
{
  switch(type)
  {
  case CardType::Desfire:
    switch(extraDataIndex)
    {
    case 0:
      hwVer = (data[8]<<8) + data[9];
      if(data[10]==0x18) size=4096;
      break;
    case 1:
      swVer = (data[8]<<8) + data[9];
      break;
    case 2:
      week = data[17];
      year = data[18];
      break;
    }
    break;
    default:
    	break;
  }
  extraDataIndex+=1;
}
//------------------------------------------------------------------------------
void NfcCard::AddIDs(uint8_t num, uint8_t* idbuff)
{
  for(uint32_t i=0;i<num;i++)
  {
    //applicationIds[(numberOfApplications*3)+(i+3)]=idbuff[i];
    numberOfApplications+=1;
  }
}
//------------------------------------------------------------------------------

