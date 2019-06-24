/*
  FILE: x_IOT.h
  Created on: 3/16/2018, by Tom Diaz
*/

#ifndef IOTGATEWAY_H
#define IOTGATEWAY_H

#include "buffer.h"
#include "httpparser.h"

class IOTGateway
{
public:
  IOTGateway(){};
  ~IOTGateway(){};
  bool IsOnline() { return associated; }
  virtual bool Post(uint8_t* data_, uint16_t len) = 0;
  virtual void Parse(BufferHandler* b) = 0;
protected:
  bool associated;
  HttpParser* parser;
private:
};

#endif /* IOTGATEWAY_H */
