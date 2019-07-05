/*
  FILE: httpparser.h
  Created on: 9/16/2017, by Tom Diaz
*/
#ifndef HTTPPARSER_H
#define HTTPPARSER_H

#include <stdint.h>
#include <buffer.h>
#include <unixtime.h>
#include <x_callback.h>

enum class MIME
{
  text_plain,
  application_json,
  application_x_www_form_urlencoded,
  application_octet_stream,
  application_msgpack
};

static const char* cType[] = 
{
  "text/plain",                                 //0
  "application/json",                           //1
  "application/x-www-form-urlencoded",          //2
  "application/octet-stream",                   //3
  "application/x-msgpack"                       //4
};

static const char* Months[]
{
  "Jan",
  "Feb",
  "Mar",
  "Apr",
  "May",
  "Jun",
  "Jul",
  "Aug",
  "Sep",
  "Oct",
  "Nov",
  "Dec",
};

class HttpParser
{
public:
  HttpParser(){ response = -1; }
  ~HttpParser(){};
  void Init(x_Callback* c);
  int16_t ParseHeader(BufferHandler* b, bool stripHeader);
  void CreateGet(BufferHandler* b, uint8_t* host, uint8_t* file);
  void CreatePost(BufferHandler* b, uint8_t* host, uint8_t* file, uint8_t* data_, 
                  uint16_t len);
  void SetMime(MIME mime_);
  void SetResponse(uint16_t r) { response = r; }
  int32_t Response();
  x_Callback* Callback() { return callback; }
protected:
  int16_t findHeaderBegin(uint8_t* data);
  int16_t findDateField(uint8_t* data);
  int16_t findHeaderEnd(uint8_t* data);
  bool extractUnixTime(uint8_t* data);
  int16_t findResponse(uint8_t* data);
  x_Callback* callback;
  int16_t response;
  uint16_t mime;
  DateTime time;
private:
};


#endif /* HTTPPARSER_H */
