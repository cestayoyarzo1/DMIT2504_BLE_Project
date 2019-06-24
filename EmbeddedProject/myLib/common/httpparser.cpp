/*
  FILE: httpparser.cpp
  Created on: 9/16/2017, by Tom Diaz
*/

#include <hal.h>
#include <httpparser.h>
#include <string.h>
#include <stdio.h>

//#define USE_HAL_DEBUG

static const char* nfile = "/";

void HttpParser::Init(x_Callback* c)
{
  callback = c;
}
//------------------------------------------------------------------------------
int16_t HttpParser::ParseHeader(BufferHandler* b, bool stripHeader)
{
  uint8_t* rxb = b->GetPointer(0);
  int16_t begin = 0;
  int16_t date = 0;
  
#ifdef USE_HAL_DEBUG
  HAL::Debug("%s\n",b->GetPointer(0));
#endif
  
  begin = findHeaderBegin(rxb);
  
  if(begin < 0)                 // Header not found
    return begin;
  else if(begin > 0)            // Header found but garbage preceeds it
  {
    b->Discard(begin);          // Remove garbage
    begin = 0;
  }
  
  if(begin == 0)
  {
    response = findResponse(rxb+9); // "HTTP/1.x _"
    date = findDateField(rxb);
    if(date > -1)
    {
      if(extractUnixTime(rxb+date))
      {
        callback->Args[0] = UnixTime::ConvertTo(&time);
        callback->Set(Set::Rtc);
      }
    }
    if(response == 200)
    {
      if(stripHeader)
      {
        // Look for header end "\r\n\r\n"
        int16_t end = findHeaderEnd(rxb);
        // Discard header, leave only message in buffer
        b->Discard(end>0?end:0);
      }
    }
    return response;
  }
  
  return -1;
}
//------------------------------------------------------------------------------
bool HttpParser::extractUnixTime(uint8_t* data)
{
   if(!strncmp("GMT",(const char*)&data[32],3)) // If date string is GMT (UTC)
   {
     // If data string adheres to RFC 1123 standard
     if(data[4] == ':' && data[9] == ',' && data[25] == ':' && data[28] == ':')
     {
       // Day number
       time.Day = ((data[11] - 0x30) * 10) + (data[12] - 0x30);
       if(!(time.Day > 0 && time.Day < 31))
         return false;  // Day is out of range
       
       // Month
       uint32_t i = 0;
       for(i = 0; i < 13; i++)
       {
         if(!strncmp(Months[i],(const char*)&data[14],3))
           break;
       }
       time.Month = i + 1;
       if(time.Month > 12)
         return false;  // Invalid month
       
       // Year
       time.Year = ((data[20] - 0x30) * 10) + (data[21] - 0x30);
       if(!(time.Year > 0 && time.Year < 100))
         return false;  // Day is out of range
       time.Year += 2000;
       
       // Hour
       time.Hour = ((data[23] - 0x30) * 10) + (data[24] - 0x30);
       if(time.Hour > 23)
         return false;  // Day is out of range
       
       // Minute
       time.Minute = ((data[26] - 0x30) * 10) + (data[27] - 0x30);
       if(time.Minute > 59)
         return false;  // Day is out of range
       
       // Second
       time.Second = ((data[29] - 0x30) * 10) + (data[30] - 0x30);
       if(time.Second > 59)
         return false;  // Day is out of range
       
       return true;
     }
   }
  return false;
}
//------------------------------------------------------------------------------
// Method looks for "DATE" at the begining of the buffer if not found returns -1
// TODO: identify first line of header (until "\r\n") then look for "HTTP" all
// across the first line, if found return location where it was found so calling
// method can strip garbage, if not found return -1
int16_t HttpParser::findDateField(uint8_t* data)
{
  uint8_t* ptr = data;
  while(ptr)
  {
    if(strncmp("Date",(char const*)ptr,4)) // if Date not found in current line
    {
      ptr = (uint8_t*)strchr((char*) ptr, '\n');
      if(ptr) // Found line end, no null char in sight
      {
        ++ptr; // Advance pointer to next line
        if(ptr[0] == '\r' && ptr[1] == '\n') // Double new line, end of header
          ptr = 0;
      }
    }
    else
      return ptr - data;
  }
  return -1;
}
//------------------------------------------------------------------------------
// Method looks for "HTTP" at the begining of the buffer if not found returns -1
// TODO: identify first line of header (until "\r\n") then look for "HTTP" all
// across the first line, if found return location where it was found so calling
// method can strip garbage, if not found return -1
int16_t HttpParser::findHeaderBegin(uint8_t* data)
{
  //uint8_t* window = data;
  const char* h0 = (const char*)data;
  
  // Header found at the begining of the buffer
  if(!strncmp("HTTP",(char const*)data,4))
    return 0;
  else
  {
    const char* h = 0;
    do
    {
      h = strchr(h0,'H');
      if(h)
      {
        if((h[1] == 'T') && (h[2] == 'T') && (h[3] == 'P'))
          return (uint8_t*)h - data;
        else
          h0 = h+1;
      }
    }
    while(h);
  }
  
  return -1;
}
//------------------------------------------------------------------------------
// Method looks for "\r\n\r\n" until infinite, dangerous, implement limit, if not
// found within limit, return -1
int16_t HttpParser::findHeaderEnd(uint8_t* data)
{
  uint8_t* back = data;
  while(true)
  {
    if(*data == '\r')
    {
      if(!strncmp("\r\n\r\n",(char const*)data,4))
      {
        data +=4;
        break;
      }
      else
        data += 2;
    }
    else
      ++data;
  }
  return (int16_t)(data - back);
}
//------------------------------------------------------------------------------
int16_t HttpParser::findResponse(uint8_t* data)
{
  int16_t res = 0;
  
  res = (data[0] - 0x30) * 100;
  res += (data[1] - 0x30) * 10;
  res += (data[2] - 0x30);
  
  return res;
}
//------------------------------------------------------------------------------
void HttpParser::SetMime(MIME mime_)
{
  mime = (uint16_t)mime_;
}
//------------------------------------------------------------------------------
void HttpParser::CreateGet(BufferHandler* b, uint8_t* host, uint8_t* file)
{
  if(host)
  {
    b->Clear();
    if(!file)
      file = (uint8_t*)nfile;
    b->SPrintf("GET %s HTTP/1.1\r\nHost: %s\r\nAccept-Encoding: \r\n\r\n",file,host);
    b->Adjust();  
  }
}
//------------------------------------------------------------------------------
void HttpParser::CreatePost(BufferHandler* b, uint8_t* host, uint8_t* file, uint8_t* data_, uint16_t len)
{
  if(host)
  {
    b->Clear();
    if(!file)
      file = (uint8_t*)nfile;
    b->SPrintf("POST %s HTTP/1.1\r\nHost: %s\r\nContent-Length: %u\r\nAccept-Encoding: gzip,deflate\r\nContent-Type: %s\r\n\r\n",
               file,host,len,cType[mime]);
    b->Adjust();
    b->AddArray(data_,len);
    
#ifdef USE_HAL_DEBUG
    HAL::Debug("%s\n",b->GetPointer(0));
#endif
  }
}
//------------------------------------------------------------------------------
int32_t HttpParser::Response()
{
  int32_t tmp = response;
  response = -1;
  return tmp;
}