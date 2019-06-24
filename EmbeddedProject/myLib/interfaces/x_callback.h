/*
FILE: x_callback.h
Created on: 6/29/2016, by Tom Diaz
*/

#ifndef X_CALLBACK_H
#define X_CALLBACK_H

#include "stdint.h"

#define MAX_CALLBACK_ARGS 6

//******************************************************************************
//Example enums, specific enums are to be provided by class extending x_Callback
/*class Process
{
public:
  enum Value
  {
    Value1
  };
};*/
//Example enums, specific enums are to be provided by class extending x_Callback
/*class Set
{
public:
  enum Value
  {
    Value2
  };
};*/
//Example enums, specific enums are to be provided by class extending x_Callback
/*class Get
{
public:
  enum Value
  {
    Value3
  };
};*/
//Example enums, specific enums are to be provided by class extending x_Callback
/*class Clear
{
public:
  enum Value
  {
    Value4
  };
};*/
//******************************************************************************
class x_Callback
{
public:
  x_Callback(){};
  ~x_Callback(){};
  virtual intptr_t Process(uint32_t msg) = 0;
  virtual intptr_t Get(uint32_t msg) = 0;
  virtual intptr_t Set(uint32_t msg) = 0;
  virtual intptr_t Clear(uint32_t msg) = 0;
  intptr_t Args[4];
protected:
  volatile bool lock[4];
};


#endif /* X_CALLBACK_H */

