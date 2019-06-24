/*
 * buffer.cpp
 *
 *  Created on: Sep 15, 2016
 *      Author: Tom Diaz
 */

#include <hal.h>	// Platform dependent
#include <buffer.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#define MAX_SEARCH_LEN 512
   
// No linkage outside this file
static uint8_t tmpBuffer[TMP_BUFFER_SIZE];
static volatile uint32_t key[4];
static uint16_t staticId = 0;
//------------------------------------------------------------------------------
BufferHandler::BufferHandler()
{
  id = staticId++;
  //---------------Default encryption key
  LoadKey((uint8_t*)"0123456789ABCDEF\0");
}
//------------------------------------------------------------------------------
int32_t BufferHandler::Find(uint8_t byte)
{  
  int32_t limit = (bufferSize > MAX_SEARCH_LEN) ? MAX_SEARCH_LEN : bufferSize;
  for(int32_t i = 0; i < limit; i++)
  {
    if(buffer[i] == byte)
      return i;
  }
  
  return -1;
}
//------------------------------------------------------------------------------
bool BufferHandler::CreateJSONPair(const char* type)
{
  uint16_t backupLength = length;
  uint16_t typeLength = strlen(type);
  uint16_t shift = strlen("{\"Type\":\"");
  shift += strlen(type);
  shift += strlen("\",\"Object\":\"");
  
  //memmove(buffer + typeLength + 12,buffer,length);
  Shift(shift);
  length = 0;
  SPrintf("{\"Type\":\"%s\",\"Object\":\"",type);   // Add JSON Fixed Formatting
  length = shift + backupLength;
  AddArray((void*)"\"}\0");                      // Close JSON string
  /*
  memcpy(tmpBuffer,buffer,length);      // Backup payload
  Clear();
  // {"Type":"i","Object":"j"}
  SPrintf("{\"Type\":\"%s\",\"Object\":\"",type);   // Add JSON Fixed Formatting
  Adjust();                             // Adjust length to JSON string length
  AddArray(tmpBuffer,backupLength);     // Add Object to JSON string
  AddArray((void*)"\"}\0");                      // Close JSON string
  */
  return true;
}
//------------------------------------------------------------------------------
void BufferHandler::CopyBuffer(BufferHandler* src, uint32_t start)
{
  memcpy(buffer+start,src->buffer,src->length);
  length = src->length;
  buffer[length] = 0;
}
//------------------------------------------------------------------------------
void BufferHandler::SPrintf(const char* format, ...)
{
  uint32_t lenx = strlen(format);
  if(lenx < TMP_BUFFER_SIZE)
  {
    //uint32_t len = 0;
    va_list arg; 
    va_start(arg, format); 
    //vsprintf((char*)buffer,format,arg);
    vsprintf((char*)tmpBuffer,format,arg);
    AddArray(tmpBuffer,strlen((const char*)tmpBuffer));
    //Adjust();
    va_end(arg);
  }
  else
    AddArray((void*)"str too long for tmpBuffer\n",27);
}
//------------------------------------------------------------------------------
void BufferHandler::SPrintf(const char* format, __Va_list l)
{
  uint32_t lenx = strlen(format);
  if(lenx < TMP_BUFFER_SIZE)
  {
    //vsprintf((char*)buffer,format,l);
    vsprintf((char*)tmpBuffer,format,l);
    AddArray(tmpBuffer,strlen((const char*)tmpBuffer));
    //Adjust();
  }
  else
    AddArray((void*)"str too long for tmpBuffer\n",27);
}
//------------------------------------------------------------------------------
void BufferHandler::SetLength(uint32_t len)
{
  length = len;
  buffer[len] = 0;
}
//------------------------------------------------------------------------------
void BufferHandler::Close()
{
  buffer[length] = 0;
}
//------------------------------------------------------------------------------
void BufferHandler::Adjust()
{
  length = strlen((char const*)buffer);
}
//------------------------------------------------------------------------------
uint32_t BufferHandler::Size()
{
  return bufferSize;
}
//------------------------------------------------------------------------------
bool BufferHandler::SimplePack(bool enc)
{
  // Format: CRC (4 bytes) + Mod (1 byte) + Payload (bufferSize - 5 bytes)
  
  // Encrypt payload and mod if requested
  if(enc)
    ToXTEA();
  
  // Make room for Mod, if not enough room then abort
  if(Shift(1))
    buffer[0] = enc ? '#' : '$'; // Add Mod, # = Encrypted, $ = Unenrypted
  else 
    return false;
  
  // Now we have Mod + (Optionally) Encrypted Payload
  
  // Calculate Mod + Payload CRC
  uint32_t crc = HAL::CRC32(buffer,length,0xffffffff);
  
  // Make room for CRC, if not enough space then abort
  if(Shift(sizeof(crc)))
  {
    // Add CRC at the begining of the buffer
    memcpy(buffer,&crc,sizeof(crc));
    // Encode whole buffer as Base64
    ToB64();
    //AddCr();
    return true;
  }
  else
    return false;
}
//------------------------------------------------------------------------------
bool BufferHandler::SimpleUnpack()
{
  // Format: CRC (4 bytes) + Mod (1 byte) + Payload (bufferSize - 5 bytes)
  
  // CRC + Mod = 5 bytes, therefore buffer in invalid or not packed if length<5
  if(length<5)
    return false;
  
  // Test for B64 valid length buffer (doesn't destroy data), if compliant then
  // decode B64, if not, return and leave data untouched.
  if(!FromB64())
    return false;
  
  // Get received CRC
  uint32_t r_crc = Get<uint32_t>(0);
  // Calculate Mod + Payload CRC
  uint32_t c_crc =  HAL::CRC32(buffer+sizeof(c_crc),
                               length-sizeof(c_crc),0xffffffff);
  
  // If received crc and calculated CRD match
  if(r_crc == c_crc)
  {
    uint8_t mod = buffer[4];            // Save Mod
    Discard(5);                       // Get rid of CRC and Mod
    // If payload is encrypted, then decrypt it
    if(mod == '#')
      FromXTEA();
    return true;
  }
  else
  {
    // If we got to this point then buffer was decoded as B64 when it may have
    // not being B64 effectively destroying data. Reverse action to recover data
    ToB64();
    return false;
  }
}
//------------------------------------------------------------------------------
uint8_t BufferHandler::getChk()
{
  uint8_t chk = 0;
  for(uint_fast8_t i=0; i<length; i++)
    chk^=buffer[i];
  return chk;
}
//------------------------------------------------------------------------------
bool BufferHandler::Shift(uint16_t len)
{
  if(length + len < bufferSize)
  {
    memmove(buffer+len,buffer,len+length);
    length += len;
    buffer[length]=0;
    return true;
  }
  return false;
}
//------------------------------------------------------------------------------
bool BufferHandler::Discard(uint16_t len)
{
  if(length >= len)
  {
    uint32_t newLength = length - len;
    memmove(buffer,buffer+len,length-len);
    memset(buffer+newLength,0,len);
    length = newLength;
    buffer[length]=0;
    return true;
  }
  return false;
}
//------------------------------------------------------------------------------
void BufferHandler::Init(uint8_t* buff,uint32_t size)
{
  buffer=buff;
  //bufferSizePadded=size;
  bufferSize=size;//bufferSizePadded-8;
  length=0;
  idx=0;
}
//------------------------------------------------------------------------------
void BufferHandler::AddCr()
{
  buffer[length++] = '\r';
}
//------------------------------------------------------------------------------
void BufferHandler::AddCrLf()
{
  buffer[length++] = '\r';
  buffer[length++] = '\n';
}
//------------------------------------------------------------------------------
void BufferHandler::Clear()
{
  idx=0;
  length=0;
  buffer[0] = 0;
  buffer[1] = 0;
  //memset(buffer,0,bufferSize);
}
//------------------------------------------------------------------------------
uint32_t BufferHandler::Length()
{
  return(length);
}
//------------------------------------------------------------------------------
uint32_t BufferHandler::Idx()
{
  return(idx);
}
//------------------------------------------------------------------------------
void BufferHandler::InsertByte(uint16_t location, uint8_t data)
{
  if(location<length)
    buffer[location] = data;
}
//------------------------------------------------------------------------------
void BufferHandler::AddArray(void* src)
{
  //uint8_t* data = (uint8_t*)src;
  uint32_t tmp = strlen((char const*)src);
  AddArray(src,tmp);
  /*uint32_t tmp = strlen((char const*)data);
  if(tmp+length<bufferSize)
  {
    memcpy(buffer+length,data,tmp);
    length+=tmp;
  }*/
}
//------------------------------------------------------------------------------
void BufferHandler::GetArray(void* dst,uint32_t start, uint32_t len)
{
  uint8_t* data = (uint8_t*)dst;
  memcpy(data,buffer + start, len);
}
//------------------------------------------------------------------------------
void BufferHandler::GetString(void* dst,uint32_t start)
{
  uint8_t* data = (uint8_t*)dst;
  uint32_t tmp = strlen((char const*)buffer + start);
  memcpy(data,buffer + start, tmp);
  data[tmp]=0;
  /*if(tmp+length<bufferSize)
  {
    memcpy(buffer+length,data,tmp);
    length+=tmp;
  }*/
}
//------------------------------------------------------------------------------
void BufferHandler::AddArray(void* src, uint32_t len)
{
  uint8_t* data = (uint8_t*)src;
  // if full src fits then add it, if not, calculate how much if src fits and
  // add it partially.
  len = (len+length<bufferSize) ? len : bufferSize - length;
  /*if(len+length<bufferSize)
  {*/
    memcpy(buffer+length,data,len);
    length+=len;
    buffer[length] = 0;
  //}
}
//------------------------------------------------------------------------------
void BufferHandler::PackForSL03x()
{
  uint8_t chk=0;
  buffer[1] = length-1;
  for(unsigned int i = 0;i<length;i++)
    chk^=buffer[i];
  Add<uint8_t>(chk);
}
//------------------------------------------------------------------------------
bool BufferHandler::UnpackFromSL03x()
{
  uint8_t chkl = 0;
  uint8_t l = buffer[0]+2;
  for(uint8_t i=2;i<=l;i++)
    chkl^=buffer[i];
  if(chkl!=buffer[1])
    return false;
  memset(tmpBuffer,0,bufferSize);
  memcpy(tmpBuffer,buffer+2,bufferSize-2);
  memset(buffer,0,bufferSize);
  memcpy(buffer,tmpBuffer,bufferSize);
  length-=2;
  return true;
}
//------------------------------------------------------------------------------
void BufferHandler::RotateLeft(uint16_t pos, uint16_t len)
{
  uint8_t* data = &buffer[pos];
  unsigned char d = data[0];
  for(int i=0;i<len-1;i++)
    data[i] = data[i+1];
  data[len-1]=d;
}
//------------------------------------------------------------------------------
void BufferHandler::Xor(uint16_t pos, uint8_t* op2, uint16_t len)
{
  uint8_t* op1 = &buffer[pos];
  for(int i=0;i<len;i++)
    op1[i]^=op2[i];
}
//------------------------------------------------------------------------------
uint8_t* BufferHandler::GetPointer(uint32_t index)
{
  if(index<bufferSize-1)
    return(&buffer[index]);
  else
    return(&buffer[0]);
}
//------------------------------------------------------------------------------
uint8_t BufferHandler::TakeInt8(volatile uint8_t *data)
{
  if(idx<length)
  {
    *data=buffer[idx++];
    return(true);
  }
  else
    Clear();
  return(false);
}
//------------------------------------------------------------------------------
void BufferHandler::ToAsciiHex()
{
  uint_fast32_t i,j;
  const char asciiHex[]="0123456789ABCDEF";
  
  memset(tmpBuffer,0,bufferSize); //Clear tmpBuffer before use
  j=0;
  for(i=0;i<length;i++)
  {
    tmpBuffer[j] = asciiHex[(buffer[i]&0xF0)>>4];
    tmpBuffer[j+1] = asciiHex[(buffer[i]&0x0F)];
    j+=2;    
  } 
  length=j;
  memcpy(buffer,tmpBuffer,length);
}
//------------------------------------------------------------------------------
void BufferHandler::PackCC1101()
{
  memset(tmpBuffer,0,bufferSize); //Clear temp buffer
  tmpBuffer[0]=length&0xff;             //Set first temp location as data length
  for(uint_fast16_t i=0;i<length;i++)   //Calc checksum of data only
    tmpBuffer[1]^=buffer[i];            //And store it in locarion 1 of temp
  memcpy(tmpBuffer+2,buffer,length);    //Copy data to temp
  tmpBuffer[length+2]=0;                //Set null character at last temp loc.
  memcpy(buffer,tmpBuffer,length+2);    //Copy back data to main buffer
  buffer[0]+=1;                         //Data length increases by 1 (CHK)
  length+=2;                            //Buffer length increases by 2 (Data Lenght and CHK)
}
//------------------------------------------------------------------------------
bool BufferHandler::UnpackCC1101()
{
  uint8_t chkl = 0;
  uint8_t l = buffer[0]+2;
  for(uint_fast16_t i=2;i<=l;i++)
    chkl^=buffer[i];
  if(chkl!=buffer[1])
    return false;
  memset(tmpBuffer,0,bufferSize);
  memcpy(tmpBuffer,buffer+2,bufferSize-2);
  memset(buffer,0,bufferSize);
  memcpy(buffer,tmpBuffer,bufferSize);
  length-=2;
  return true;
}
//------------------------------------------------------------------------------
void BufferHandler::encBlock(uint32_t* data)
{
    uint32_t v0=data[0], v1=data[1], sum=0, delta=0x9E3779B9;
    for (uint32_t i=0; i < xteaRounds; i++)
    {
        v0 += (((v1 << 4) ^ (v1 >> 5)) + v1) ^ (sum + key[sum & 3]);
        sum += delta;
        v1 += (((v0 << 4) ^ (v0 >> 5)) + v0) ^ (sum + key[(sum>>11) & 3]);
    }
    data[0]=v0;
    data[1]=v1;
}
//------------------------------------------------------------------------------
void BufferHandler::decBlock(uint32_t* data)
{
    uint32_t v0=data[0], v1=data[1], delta=0x9E3779B9, sum=delta*xteaRounds;
    for (uint32_t i=0; i < xteaRounds; i++)
    {
        v1 -= (((v0 << 4) ^ (v0 >> 5)) + v0) ^ (sum + key[(sum>>11) & 3]);
        sum -= delta;
        v0 -= (((v1 << 4) ^ (v1 >> 5)) + v1) ^ (sum + key[sum & 3]);
    }
    data[0]=v0;
    data[1]=v1;
}
//------------------------------------------------------------------------------
void BufferHandler::ToXTEA()
{
  if(length%8)
    pad((8*(length/8))+8);

  for(uint32_t i=0; i<length;i+=8)
    encBlock((uint32_t*)&buffer[i]);
}
//------------------------------------------------------------------------------
void BufferHandler::FromXTEA()
{
  for(uint32_t i=0; i<length;i+=8)
    decBlock((uint32_t*)&buffer[i]);
}
//------------------------------------------------------------------------------
void BufferHandler::pad(uint32_t padding)
{
  if((padding>length)&&(padding<bufferSize))
    length=padding;
}
//-----------------------------------------------------------------------------
uint8_t BufferHandler::toAsciiChar(int8_t data)
{
  if(data>=0 && data<=9)
    return data+0x30;
  else
    return data+0x37;
}
//----------------------------------------------------------------decodeBase64()
bool BufferHandler::FromB64()
{
  uint32_t c0=0,c1=0,c2=0,c3=0;
  uint32_t padding = 0;
  uint32_t j = 0; //idx for tmpBuffer

  /*if(buffer[0]=='@')
    return false;*/

  //length=strlen((const char*)buffer);
/*
  (buffer[length-2]=='=')?padding+=2:((buffer[length-1]=='=')?padding+=1:padding=0);

  memset(tmpBuffer,0,TMP_BUFFER_SIZE); //Clear tmpBuffer before use
*/
  if(!(length%4))           //Buffer length has to be a multiple of four
  {
    (buffer[length-2]=='=')?padding+=2:((buffer[length-1]=='=')?padding+=1:padding=0);
    memset(tmpBuffer,0,TMP_BUFFER_SIZE); //Clear tmpBuffer before use
    
    for(uint32_t i=0; i<length; i+=4)
    {
      c0=fromB64Char(buffer[i]);
      c1=fromB64Char(buffer[i+1]);
      c2=fromB64Char(buffer[i+2]);
      c3=fromB64Char(buffer[i+3]);
      
      // If an non B64 char is detected
      if((c0==0xff) || (c1==0xff) || (c2==0xff) || (c3==0xff))
        return false;
      
      tmpBuffer[j+(2*j)] = (uint8_t)(c0 << 2 | c1 >> 4);
      tmpBuffer[j+(2*j)+1] = (uint8_t)(c1 << 4 | c2 >> 2);
      tmpBuffer[j+(2*j)+2] = (uint8_t)(((c2 << 6) & 0xc0) | c3);
      j+=1;
    }
    length = j*3;
    memcpy(buffer,tmpBuffer,length);
    length -= padding;
    return true;
  }
  else
    return false;
}
//----------------------------------------------------encodeBase64(BYTE* cmdStr)
bool BufferHandler::ToB64()
{
  const char base64chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  uint32_t n = 0;
  uint32_t padCount = length % 3;
  uint32_t n0, n1, n2, n3;
  
  n0 = length;
  while(n0%3) ++n0;     // Round length to a multiple of 3
  n1 = 4*(n0/3);        // Calculate length after encoding
  if(n1 > bufferSize)   // If greater than bufferSize
    return false;       // Don't do anything.
  
  n0 = 0;
  n1 = 1;

  uint32_t j = 0; //idx for tmpBuffer

  memset(tmpBuffer,0,bufferSize); //Clear tmpBuffer before use

   for (uint32_t x = 0; x < length; x += 3)
   {
      /* these three 8-bit (ASCII) characters become one 24-bit number */
      n = buffer[x] << 16;

      if((x+1) < length)
         n += buffer[x+1] << 8;

      if((x+2) < length)
         n += buffer[x+2];

      /* this 24-bit number gets separated into four 6-bit numbers */
      n0 = (uint8_t)((n >> 18) & 63);
      n1 = (uint8_t)((n >> 12) & 63);
      n2 = (uint8_t)((n >> 6) & 63);
      n3 = (uint8_t)(n & 63);

      /*
       * if we have one byte available, then its encoding is spread
       * out over two characters
       */
      tmpBuffer[j++] = base64chars[n0];
      tmpBuffer[j++] = base64chars[n1];
      /*
       * if we have only two bytes available, then their encoding is
       * spread out over three chars
       */
      if((x+1) < length)
         tmpBuffer[j++] = base64chars[n2];
      /*
       * if we have all three bytes available, then their encoding is spread
       * out over four characters
       */
      if((x+2) < length)
         tmpBuffer[j++] = base64chars[n3];
   }

   if (padCount > 0)
   {
      for (; padCount < 3; padCount++)
        tmpBuffer[j++] = '=';
   }

   length=j;
   memcpy(buffer,tmpBuffer,length);
   buffer[length]=0;
   return true;
}
//-----------------------------------------------getValueFromTable(BYTE b64char)
uint8_t BufferHandler::fromB64Char(uint8_t b64char)
{
  if((b64char>='A') && (b64char<='Z'))
    return(b64char - 'A');

  if((b64char>='a') && (b64char<='z'))
    return(b64char - 'a' + 26);

  if((b64char>='0') && (b64char<='9'))
    return(b64char - '0' + 52);

  if(b64char=='+')
    return(62);

  if(b64char=='/')
    return(63);
  
  if(b64char=='=')
    return(0);

  return 0xff;
}
//------------------------------------------------------------------------------
void BufferHandler::LoadKey(uint8_t* tmp)
{
  for (uint32_t i = 0; i < 4; i++)
  {
    key[i] = (uint32_t)(tmp[(i * 4)] << 24);
    key[i] += (uint32_t)(tmp[(i * 4)+1] << 16);
    key[i] += (uint32_t)(tmp[(i * 4)+2] << 8);
    key[i] += (uint32_t)(tmp[(i * 4)+3]);
  }
}
//------------------------------------------------------------------------------
