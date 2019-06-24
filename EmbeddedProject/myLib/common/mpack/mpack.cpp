/*
  FILE: mpack.cpp
  Created on: 9/12/2017, by Tom Diaz
*/

#include <mpack.h>
#include <string.h>
#include <buffer.h>

bool cmp_read(struct cmp_ctx_s *ctx, void *data, size_t limit)
{
  memcpy(data,ctx->buf,limit);
  ctx->buf = ((uint8_t*)ctx->buf) + limit;
  
  return true;
}

bool cmp_skip(struct cmp_ctx_s *ctx, size_t count)
{
  ctx->buf = ((uint8_t*)ctx->buf) + count;
  return true;
}

size_t cmp_write(struct cmp_ctx_s *ctx, const void *data, size_t count)
{
  //memcpy(ctx->buf,data,count);
  //ctx->buf = ((uint8_t*)ctx->buf) + count;
  
  BufferHandler* handler = (BufferHandler*)ctx->handler;
  handler->AddArray((void*)data,count);
  ctx->buf = ((uint8_t*)ctx->buf) + count;
  
  return count;
}

//------------------------------------------------------------------------------
/*void MessagePack::Init(uint8_t* buff, uint32_t len)
{
  buffer = buff;
  bufferLength = len;
  cmp_init(&ctx,buffer,cmp_read,cmp_skip,cmp_write);
}*/
//------------------------------------------------------------------------------
void MessagePack::Init(void* handler_)
{
  ctx.handler = handler_;
  ctx.buf = ((BufferHandler*)ctx.handler)->GetPointer(0);
  ctx.read = cmp_read;
  ctx.skip = cmp_skip;
  ctx.write = cmp_write;
}
//------------------------------------------------------------------------------
void MessagePack::Rewind()
{
  //ctx.buf = buffer;
  ctx.buf = ((BufferHandler*)ctx.handler)->GetPointer(0);
}
//------------------------------------------------------------------------------
uint32_t MessagePack::GetSize()
{
  //return ((uint32_t)ctx.buf - (uint32_t)buffer);
  //return ((uint32_t)ctx.buf - (uint32_t)((BufferHandler*)ctx.handler)->GetPointer(0));
  BufferHandler* handler = (BufferHandler*)ctx.handler;
  return handler->Length();
}
//------------------------------------------------------------------------------
uint8_t* MessagePack::GetBuffer()
{
  //return buffer;
  return ((BufferHandler*)ctx.handler)->GetPointer(0);
}
//------------------------------------------------------------------------------
bool MessagePack::WriteString(const char* str)
{
  uint32_t len = strlen(str);
  lastAns = cmp_write_str_v4(&ctx,str,len);
  return lastAns;
}
//------------------------------------------------------------------------------
bool MessagePack::WriteString(uint8_t* arr, uint32_t len)
{
  lastAns = cmp_write_str_v4(&ctx,(const char*)arr,len);
  return lastAns;
}
//------------------------------------------------------------------------------
bool MessagePack::WriteInteger(int32_t val)
{
  lastAns = cmp_write_integer(&ctx, val);
  return lastAns;
}
//------------------------------------------------------------------------------
bool MessagePack::ReadInteger(int32_t* val)
{
  lastAns = cmp_read_integer(&ctx, (int64_t*)val);
  return lastAns;
}
//------------------------------------------------------------------------------
bool MessagePack::WriteUInteger(uint32_t val)
{
  lastAns = cmp_write_uinteger(&ctx, val);
  return lastAns;
}
//------------------------------------------------------------------------------
bool MessagePack::WriteDecimal(double val)
{
  lastAns = cmp_write_decimal(&ctx, val);
  return lastAns;
}
//------------------------------------------------------------------------------
bool MessagePack::WriteArray(uint32_t size)
{
  lastAns = cmp_write_array(&ctx,size);
  return lastAns;
}
//------------------------------------------------------------------------------
bool MessagePack::WriteMap(uint32_t size)
{
  lastAns = cmp_write_map(&ctx, size);
  return lastAns;
}
//------------------------------------------------------------------------------
bool MessagePack::WriteBinary(void* data, uint32_t size)
{
  lastAns = cmp_write_bin(&ctx, (char const*)data, size);
  return lastAns;
}
//------------------------------------------------------------------------------
bool MessagePack::ReadStr(uint8_t* str, uint8_t bSize)
{
  uint32_t ptr = bSize;
  lastAns = cmp_read_str(&ctx,(char*)str,&ptr); 
  return lastAns;
}
static cmp_object_t obj;
//------------------------------------------------------------------------------
bool MessagePack::Skip()
{
  lastAns = cmp_skip_object(&ctx,&obj); 
  return lastAns;
}
//------------------------------------------------------------------------------
bool MessagePack::ReadMap()
{
  lastAns = cmp_read_map(&ctx, &objSize);
  return lastAns;
}
//------------------------------------------------------------------------------
bool MessagePack::ReadObject(uint8_t* data, int32_t* size)
{
  lastAns = cmp_read_bin(&ctx, data, (uint32_t*)size);
  *size = lastAns ? *size : -1;
  return lastAns;
}
//------------------------------------------------------------------------------
bool MessagePack::ReadArray(uint32_t* size)
{
  lastAns = cmp_read_array(&ctx, size);
  return lastAns;
}
//------------------------------------------------------------------------------